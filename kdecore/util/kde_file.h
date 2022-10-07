/*
   This file is part of the KDE libraries
   Copyright (C) 2001 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2004 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _KDE_FILE_H_
#define _KDE_FILE_H_

/**
 * \file kde_file.h
 * \brief This file provides portable defines for file support.
 *
 * Use the KDE_xxx defines instead of the normal C
 * functions and structures.
 * \since 3.3
 */

#include <utime.h>
#include <qplatformdefs.h>
#include <qfile.h>
#include <kdecore_export.h>

/* definitions that are for compatibility, will be removed in the future */
#define KDE_stat                QT_STAT
#define KDE_lstat               QT_LSTAT
#define KDE_fstat               QT_FSTAT
#define KDE_open                QT_OPEN
#define KDE_fopen               QT_FOPEN
#define KDE_lseek               QT_LSEEK
#define KDE_fseek               QT_FSEEK
#define KDE_ftell               QT_FTELL
#define KDE_fgetpos             QT_FGETPOS
#define KDE_fsetpos             QT_FSETPOS
#define KDE_readdir             QT_READDIR
#define KDE_struct_stat         QT_STATBUF
#define KDE_struct_dirent       QT_DIRENT

/* functions without 64-bit version but wrapped for compatibility reasons */
#define KDE_rename          ::rename
#define KDE_mkdir           ::mkdir
#define KDE_fdopen          ::fdopen
#define KDE_signal          ::signal

namespace KDE
{
    /** replacement for ::access() to handle filenames in a platform independent way */
    KDECORE_EXPORT int access(const QString &path, int mode);
    /** replacement for ::chmod() to handle filenames in a platform independent way */
    KDECORE_EXPORT int chmod(const QString &path, mode_t mode);
    /** replacement for ::fopen()/::fopen64() to handle filenames in a platform independent way */
    KDECORE_EXPORT FILE *fopen(const QString &pathname, const char *mode);
    /** replacement for ::lstat()/::lstat64() to handle filenames in a platform independent way */
    KDECORE_EXPORT int lstat(const QString &path, KDE_struct_stat *buf);
    /** replacement for ::mkdir() to handle pathnames in a platform independent way */
    KDECORE_EXPORT int mkdir(const QString &pathname, mode_t mode);
    /** replacement for ::open()/::open64() to handle filenames in a platform independent way */
    KDECORE_EXPORT int open(const QString &pathname, int flags, mode_t mode = 0);
    /** replacement for ::rename() to handle pathnames in a platform independent way */
    KDECORE_EXPORT int rename(const QString &in, const QString &out);
    /** replacement for ::stat()/::stat64() to handle filenames in a platform independent way */
    KDECORE_EXPORT int stat(const QString &path, KDE_struct_stat *buf);
    /** replacement for ::utime() to handle filenames in a platform independent way */
    KDECORE_EXPORT int utime(const QString &filename, struct utimbuf *buf);

    inline int access(const QString &path, int mode)
    {
        const QByteArray encodedpath(QFile::encodeName(path));
        return ::access(encodedpath.constData(), mode);
    }

    inline int chmod(const QString &path, mode_t mode)
    {
        const QByteArray encodedpath(QFile::encodeName(path));
        return ::chmod(encodedpath.constData(), mode);
    }

    inline FILE *fopen(const QString &pathname, const char *mode)
    {
        const QByteArray encodedpathname(QFile::encodeName(pathname));
        return QT_FOPEN(encodedpathname.constData(), mode);
    }

    inline int lstat(const QString &path, KDE_struct_stat *buf)
    {
        const QByteArray encodedpath(QFile::encodeName(path));
        return QT_LSTAT(encodedpath.constData(), buf);
    }

    inline int mkdir(const QString &pathname, mode_t mode)
    {
        const QByteArray encodedpathname(QFile::encodeName(pathname));
        return ::mkdir(encodedpathname.constData(), mode);
    }

    inline int open(const QString &pathname, int flags, mode_t mode)
    {
        const QByteArray encodedpathname(QFile::encodeName(pathname));
        return QT_OPEN(encodedpathname.constData(), flags, mode);
    }

    inline int stat(const QString &path, KDE_struct_stat *buf)
    {
        const QByteArray encodedpath(QFile::encodeName(path));
        return QT_STAT(encodedpath.constData(), buf);
    }

    inline int utime(const QString &filename, struct utimbuf *buf)
    {
        const QByteArray encodedfilename(QFile::encodeName(filename));
        return ::utime(encodedfilename.constData(), buf);
    }
}

#define KPATH_SEPARATOR ':'
#define KDIR_SEPARATOR '/' /* faster than QDir::separator() */

#endif /* _KDE_FILE_H_ */
