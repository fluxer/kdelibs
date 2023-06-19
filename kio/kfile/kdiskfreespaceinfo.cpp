/*
 *  kdiskfreespaceinfo.h
 *
 *  Copyright 2008 Sebastian Trug <trueg@kde.org>
 *
 *  Based on kdiskfreespace.h
 *  Copyright 2007 David Faure <faure@kde.org>
 *  Copyright 2008 Dirk Mueller <mueller@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kdiskfreespaceinfo.h"

#include <QSharedData>
#include <QFile>

#include <kmountpoint.h>
#include <kdebug.h>

#include <sys/statvfs.h>
#include <errno.h>

class KDiskFreeSpaceInfo::Private : public QSharedData
{
public:
    Private()
        : valid(false),
        size(0),
        available(0)
    {
    }

    bool valid;
    QString mountPoint;
    KIO::filesize_t size;
    KIO::filesize_t available;
};


KDiskFreeSpaceInfo::KDiskFreeSpaceInfo()
    : d(new Private())
{
}

KDiskFreeSpaceInfo::KDiskFreeSpaceInfo(const KDiskFreeSpaceInfo &other)
{
    d = other.d;
}

KDiskFreeSpaceInfo::~KDiskFreeSpaceInfo()
{
}

KDiskFreeSpaceInfo& KDiskFreeSpaceInfo::operator=(const KDiskFreeSpaceInfo &other)
{
    d = other.d;
    return *this;
}

bool KDiskFreeSpaceInfo::isValid() const
{
    return d->valid;
}

QString KDiskFreeSpaceInfo::mountPoint() const
{
    return d->mountPoint;
}

KIO::filesize_t KDiskFreeSpaceInfo::size() const
{
    return d->size;
}

KIO::filesize_t KDiskFreeSpaceInfo::available() const
{
    return d->available;
}

KIO::filesize_t KDiskFreeSpaceInfo::used() const
{
    return (d->size - d->available);
}

KDiskFreeSpaceInfo KDiskFreeSpaceInfo::freeSpaceInfo(const QString &path)
{
    KDiskFreeSpaceInfo info;

    // determine the mount point
    KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByPath(path);
    if (mp) {
        info.d->mountPoint = mp->mountPoint();
    }

    struct statvfs statvfs_buf;
    // Prefer mountPoint if available, so that it even works with non-existing files.
    const QString pathArg = (info.d->mountPoint.isEmpty() ? path : info.d->mountPoint);
    const QByteArray pathArgBytes = QFile::encodeName(pathArg);
    if (::statvfs(pathArgBytes.constData(), &statvfs_buf) == 0) {
        const quint64 blksize = quint64(statvfs_buf.f_frsize); // cast to avoid overflow
        info.d->available = (statvfs_buf.f_bavail * blksize);
        info.d->size = (statvfs_buf.f_blocks * blksize);
        info.d->valid = true;
    } else {
        const int savederrno = errno;
        kWarning() << qt_error_string(savederrno);
    }

    return info;
}
