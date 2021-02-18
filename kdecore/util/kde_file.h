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
#include <kdecore_export.h>

/* added not for Solaris and OpenSolaris platforms */

#if (defined _LFS64_LARGEFILE) && (defined _LARGEFILE64_SOURCE) && (!defined _GNU_SOURCE) && (!defined __sun)
/*
 * This section provides portable defines for large file support.
 * To use this you must compile your code with _LARGEFILE64_SOURCE
 * defined and use the KDE_xxx defines instead of the normal
 * C functions and structures.
 *
 * Please note that not every platform supports 64 bit file structures,
 * in that case the normal 32 bit functions will be used.
 *
 * @see http://www.suse.de/~aj/linux_lfs.html
 * @see http://ftp.sas.com/standards/large.file/xopen/x_open.05Mar96.html
 *
 * KDE makes use of the "Transitional Extensions" since we can not ensure
 * that all modules and libraries used by KDE will be compiled with
 * 64-bit support.
 * (A.3.2.3 Mixed API and Compile Environments within a Single Process)
 */
#define KDE_sendfile            ::sendfile64

#else /* !_LFS64_LARGEFILE */

/*
 * This section defines portable defines for standard file support.
 */
#define KDE_sendfile            ::sendfile

#endif /* !_LFS64_LARGEFILE */

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

#include <QtCore/QFile>

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
    return ::access( QFile::encodeName(path).constData(), mode );
  }
  inline int chmod(const QString &path, mode_t mode)
  {
    return ::chmod( QFile::encodeName(path).constData(), mode );
  }
  inline FILE *fopen(const QString &pathname, const char *mode)
  {
    return QT_FOPEN( QFile::encodeName(pathname).constData(), mode );
  }
  inline int lstat(const QString &path, KDE_struct_stat *buf)
  {
    return QT_LSTAT( QFile::encodeName(path).constData(), buf );
  }
  inline int mkdir(const QString &pathname, mode_t mode)
  {
    return ::mkdir( QFile::encodeName(pathname).constData(), mode );
  }
  inline int open(const QString &pathname, int flags, mode_t mode)
  {
    return QT_OPEN( QFile::encodeName(pathname).constData(), flags, mode );
  }
  inline int rename(const QString &in, const QString &out)
  {
    return ::rename( QFile::encodeName(in).constData(), QFile::encodeName(out).constData() );
  }
  inline int stat(const QString &path, KDE_struct_stat *buf)
  {
    return QT_STAT( QFile::encodeName(path).constData(), buf );
  }
  inline int utime(const QString &filename, struct utimbuf *buf)
  {
    return ::utime( QFile::encodeName(filename).constData(), buf );
  }
}

#ifndef O_BINARY
#define O_BINARY 0 /* for open() */
#endif
#define KPATH_SEPARATOR ':'
#define KDIR_SEPARATOR '/' /* faster than QDir::separator() */

#endif /* _KDE_FILE_H_ */
