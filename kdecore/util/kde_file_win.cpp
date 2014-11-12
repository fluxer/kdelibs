/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2009 Christian Ehrlicher <ch.ehrlicher@gmx.de>

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

// needed for _wstat64
#define __MSVCRT_VERSION__ 0x601

#include "kde_file.h"

#include <QtCore/QFile>
#include <errno.h>

#include <sys/utime.h>
#include <sys/stat.h>
#include <wchar.h>
#define CONV(x) ((wchar_t*)x.utf16())

/** @internal, from kdewin32 lib */
static int kdewin_fix_mode_string(char *fixed_mode, const char *mode)
{
	if (strlen(mode)<1 || strlen(mode)>3) {
        errno = EINVAL;
		return 1;
    }

	strncpy(fixed_mode, mode, 3);
	if (fixed_mode[0]=='b' || fixed_mode[1]=='b' || fixed_mode[0]=='t' || fixed_mode[1]=='t')
		return 0;
	/* no 't' or 'b': append 'b' */
	if (fixed_mode[1]=='+') {
		fixed_mode[1]='b';
		fixed_mode[2]='+';
		fixed_mode[3]=0;
	}
	else {
		fixed_mode[1]='b';
		fixed_mode[2]=0;
	}
	return 0;
}

/** @internal */
static int kdewin_fix_flags(int flags)
{
	if ((flags & O_TEXT) == 0 && (flags & O_BINARY) == 0)
		return flags | O_BINARY;
	return flags;
}

/* from kdefakes library
   Generate a unique temporary directory name from TEMPLATE.

   TEMPLATE has the form:

   <path>/ccXXXXXX


   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the filename unique.

   Returns a file descriptor open on the file for reading and writing.  */

QString mkdtemp_QString (const QString &_template)
{
  static const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  char XXXXXX[7];
  int value;

  if ( !_template.endsWith(QLatin1String("XXXXXX")) )
      return QString();

  strcpy(XXXXXX, "XXXXXX");
  const QString tmpl = _template.left(_template.length() - 6);

  value = rand();
  for (int count = 0; count < 256; ++count)
  {
      int v = value;

      /* Fill in the random bits.  */
      XXXXXX[0] = letters[v % 62];
      v /= 62;
      XXXXXX[1] = letters[v % 62];
      v /= 62;
      XXXXXX[2] = letters[v % 62];
      v /= 62;
      XXXXXX[3] = letters[v % 62];
      v /= 62;
      XXXXXX[4] = letters[v % 62];
      v /= 62;
      XXXXXX[5] = letters[v % 62];

      /* This is a random value.  It is only necessary that the next
         TMP_MAX values generated by adding 7777 to VALUE are different
         with (module 2^32).  */
      value += 7777;

      const QString tmp = tmpl + QString::fromLatin1( XXXXXX );
      if (!KDE::mkdir(tmp,0700))
          return tmp;
  }
  return QString();
}

namespace KDE
{
  int access(const QString &path, int mode)
  {
    int x_mode = 0;
    // X_OK gives an assert on msvc2005 and up - use stat() instead
    if( ( mode & X_OK ) == X_OK ) {
        KDE_struct_stat st;
        if( KDE::stat( path, &st ) != 0 )
          return 1;
        if( ( st.st_mode & S_IXUSR ) != S_IXUSR )
          return 1;
    }
    mode &= ~X_OK;
    return _waccess( CONV(path), mode );
  }

  int chmod(const QString &path, mode_t mode)
  {
    return _wchmod( CONV(path), mode );
  }

  FILE *fopen(const QString &pathname, const char *mode)
  {
    return _wfopen( CONV(pathname), CONV(QString::fromLatin1( mode )) );
  }

  int lstat(const QString &path, KDE_struct_stat *buf)
  {
    return KDE::stat( path, buf );
  }

  int mkdir(const QString &pathname, mode_t)
  {
    return _wmkdir( CONV(pathname) );
  }

  int open(const QString &pathname, int flags, mode_t mode)
  {
    return _wopen( CONV(pathname), kdewin_fix_flags(flags), mode );
  }

  int rename(const QString &in, const QString &out)
  {
    // better than :waccess/_wunlink/_wrename
#ifndef _WIN32_WCE
    bool ok = ( MoveFileExW( CONV(in), CONV(out),
                             MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED ) != 0 );
#else
    bool ok = ( MoveFileW( CONV(in), CONV(out)) != 0 );
#endif
	return ok ? 0 : -1;
  }

  int stat(const QString &path, KDE_struct_stat *buf)
  {
    int result;
#ifdef Q_CC_MSVC
#ifndef _WIN32_WCE
    struct _stat64 s64;
#else
    struct stat st;
#endif
#else
    struct __stat64 s64;
#endif
    const int len = path.length();
    if ( (len==2 || len==3) && path[1]==QLatin1Char(':') && path[0].isLetter() ) {
    	/* 1) */
        QString newPath(path);
    	if (len==2)
    		newPath += QLatin1Char('\\');
#ifndef _WIN32_WCE
    	result = _wstat64( CONV(newPath), &s64 );
#else
    	result = wstat( CONV(newPath), &st );
#endif
    } else
    if ( len > 1 && (path.endsWith(QLatin1Char('\\')) || path.endsWith(QLatin1Char('/'))) ) {
    	/* 2) */
        const QString newPath = path.left( len - 1 );
#ifndef _WIN32_WCE
    	result = _wstat64( CONV(newPath), &s64 );
#else
    	result = wstat( CONV(newPath), &st );
#endif
    } else {
        //TODO: is stat("/") ok?
#ifndef _WIN32_WCE
        result = _wstat64( CONV(path), &s64 );
#else
    	result = wstat( CONV(path), &st );
#endif
    }
    if( result != 0 )
        return result;
    // KDE5: fixme!
#ifndef _WIN32_WCE
    buf->st_dev   = s64.st_dev;
    buf->st_ino   = s64.st_ino;
    buf->st_mode  = s64.st_mode;
    buf->st_nlink = s64.st_nlink;
#else
    buf->st_dev   = st.st_dev;
    buf->st_ino   = st.st_ino;
    buf->st_mode  = st.st_mode;
    buf->st_nlink = st.st_nlink;
#endif
    buf->st_uid   = -2; // be in sync with Qt4
    buf->st_gid   = -2; // be in sync with Qt4
#ifndef _WIN32_WCE
    buf->st_rdev  = s64.st_rdev;
    buf->st_size  = s64.st_size;
    buf->st_atime = s64.st_atime;
    buf->st_mtime = s64.st_mtime;
    buf->st_ctime = s64.st_ctime;
#else
    buf->st_rdev  = st.st_rdev;
    buf->st_size  = st.st_size;
    buf->st_atime = st.st_atime;
    buf->st_mtime = st.st_mtime;
    buf->st_ctime = st.st_ctime;
#endif
    return result;
  }
  int utime(const QString &filename, struct utimbuf *buf)
  {
#ifndef _WIN32_WCE
    return _wutime( CONV(filename), (struct _utimbuf*)buf );
#else
    return _wutime( CONV(filename), (struct utimbuf*)buf );
#endif
  }
};
