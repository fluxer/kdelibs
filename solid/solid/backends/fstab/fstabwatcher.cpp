/*
    Copyright 2010 Mario Bensi <mbensi@ipsquad.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "fstabwatcher.h"
#include "soliddefs_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QSocketNotifier>
#include <QtCore/QFile>
#include <QtCore/QStringList>

using namespace Solid::Backends::Fstab;

Q_GLOBAL_STATIC(FstabWatcher, globalFstabWatcher)

#ifdef Q_OS_SOLARIS
#define FSTAB "/etc/vfstab"
#define MTAB "/etc/mnttab"
#else
#define FSTAB "/etc/fstab"
#define MTAB "/etc/mtab"
#endif

FstabWatcher::FstabWatcher()
    : m_fileSystemWatcher(new QFileSystemWatcher(this))
{
    m_mtabFile = new QFile(MTAB, this);
    if (m_mtabFile && m_mtabFile->readLink().startsWith("/proc/")
        && m_mtabFile->open(QIODevice::ReadOnly) ) {

        m_mtabSocketNotifier = new QSocketNotifier(m_mtabFile->handle(),
                QSocketNotifier::Exception, this);
        connect(m_mtabSocketNotifier,
                SIGNAL(activated(int)), this, SIGNAL(mtabChanged()) );
    } else {
        m_fileSystemWatcher->addPath(MTAB);
    }

    m_fileSystemWatcher->addPath(FSTAB);
    connect(m_fileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged(QString)));
}

FstabWatcher::~FstabWatcher()
{
    m_fileSystemWatcher->deleteLater();
}

FstabWatcher *FstabWatcher::instance()
{
    return globalFstabWatcher();
}


void FstabWatcher::onFileChanged(const QString &path)
{
    if (path == MTAB) {
        emit mtabChanged();
        if (!m_fileSystemWatcher->files().contains(MTAB)) {
            m_fileSystemWatcher->addPath(MTAB);
        }
    }
    if (path == FSTAB) {
        emit fstabChanged();
        if (!m_fileSystemWatcher->files().contains(FSTAB)) {
            m_fileSystemWatcher->addPath(FSTAB);
        }
    }
}


