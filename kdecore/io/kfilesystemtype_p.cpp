/*
   This file is part of the KDE libraries
   Copyright (c) 2011 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2.1 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kfilesystemtype_p.h"
#include <QFile>
#include <QDebug>
//#include <errno.h>

inline KFileSystemType::Type kde_typeFromName(const char *name)
{
    if (qstrncmp(name, "nfs", 3) == 0
        || qstrncmp(name, "autofs", 6) == 0
        || qstrncmp(name, "cachefs", 7) == 0
        || qstrncmp(name, "fuse.sshfs", 10) == 0
        || qstrncmp(name, "xtreemfs@", 9) == 0) // #178678
        return KFileSystemType::Nfs;
    if (qstrncmp(name, "fat", 3) == 0
        || qstrncmp(name, "vfat", 4) == 0
        || qstrncmp(name, "msdos", 5) == 0)
        return KFileSystemType::Fat;
    if (qstrncmp(name, "cifs", 4) == 0
        || qstrncmp(name, "smbfs", 5) == 0)
        return KFileSystemType::Smb;
    if (qstrncmp(name, "ramfs", 5) == 0)
        return KFileSystemType::Ramfs;

    return KFileSystemType::Other;
}

#if defined(Q_OS_FREEBSD) || defined(Q_OS_DRAGONFLY) || defined(Q_OS_OPENBSD)
# include <sys/param.h>
# include <sys/mount.h>

KFileSystemType::Type determineFileSystemTypeImpl(const QByteArray& path)
{
    struct statfs buf;
    if (statfs(path.constData(), &buf) != 0)
        return KFileSystemType::Unknown;
    return kde_typeFromName(buf.f_fstypename);
}

#elif defined(Q_OS_LINUX) || defined(Q_OS_HURD)
# include <sys/vfs.h>
# include <linux/magic.h>
// LSB 3.2 has statfs in sys/statfs.h, sys/vfs.h is just an empty dummy header
# include <sys/statfs.h>

# ifndef AUTOFSNG_SUPER_MAGIC
#  define AUTOFSNG_SUPER_MAGIC  0x7d92b1a0
# endif
# ifndef FUSE_SUPER_MAGIC
#  define FUSE_SUPER_MAGIC     0x65735546
# endif

// Reverse-engineering without C++ code:
// strace stat -f /mnt 2>&1|grep statfs|grep mnt, and look for f_type

KFileSystemType::Type determineFileSystemTypeImpl(const QByteArray& path)
{
    struct statfs buf;
    if (statfs(path.constData(), &buf) != 0) {
        //kDebug() << path << errno << strerror(errno);
        return KFileSystemType::Unknown;
    }

    // TODO could be anything. Need to use statfs() to find out more.
    if (buf.f_type == NFS_SUPER_MAGIC || buf.f_type == AUTOFS_SUPER_MAGIC || buf.f_type == FUSE_SUPER_MAGIC) {
        return KFileSystemType::Nfs;
    } else if (buf.f_type == SMB_SUPER_MAGIC) {
        return KFileSystemType::Smb;
    } else if (buf.f_type == MSDOS_SUPER_MAGIC) {
        return KFileSystemType::Fat;
    } else if (buf.f_type == RAMFS_MAGIC) {
        return KFileSystemType::Ramfs;
    }
    return KFileSystemType::Other;
}

#elif defined(Q_OS_SOLARIS) || defined(Q_OS_NETBSD)
# include <sys/statvfs.h>

KFileSystemType::Type determineFileSystemTypeImpl(const QByteArray& path)
{
    struct statvfs buf;
    if (statvfs(path.constData(), &buf) != 0)
        return KFileSystemType::Unknown;
#if defined(Q_OS_NETBSD)
    return kde_typeFromName(buf.f_fstypename);
#else
    return kde_typeFromName(buf.f_basetype);
#endif
}
#endif

KFileSystemType::Type KFileSystemType::fileSystemType(const QString& path)
{
    return determineFileSystemTypeImpl(QFile::encodeName(path));
}
