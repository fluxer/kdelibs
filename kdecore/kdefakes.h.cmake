/* This file is part of the KDE libraries
   Copyright (c) 2006 The KDE Project

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

#ifndef KDEFAKES_H
#define KDEFAKES_H

/* This file defines the prototypes for a few (C library) functions for 
   platforms which either
   1) have those functions, but lack the prototypes in their header files.
   2) don't have those functions, in which case kdecore provides them

   You should include this file in any .cpp file that uses any one of these 
   functions:
     strlcat, strlcpy, 
     setenv, unsetenv, 
     usleep, initgroups, 
     mkdtemp (this is for KTempDir itself, prefer using KTempDir everywhere else)
     mkstemp, mkstemps (prefer to use QTemporaryfile instead)
     trunc
     getgrouplist
*/

#cmakedefine HAVE_STRLCAT_PROTO 1
#if !defined(HAVE_STRLCAT_PROTO)
extern "C++" unsigned long strlcat(char*, const char*, unsigned long);
#endif

#cmakedefine HAVE_STRLCPY_PROTO 1
#if !defined(HAVE_STRLCPY_PROTO)
extern "C++" unsigned long strlcpy(char*, const char*, unsigned long);
#endif

#cmakedefine HAVE_SETENV_PROTO 1
#if !defined(HAVE_SETENV_PROTO)
extern "C++" int setenv (const char *, const char *, int);
#endif

#cmakedefine HAVE_UNSETENV_PROTO 1
#if !defined(HAVE_UNSETENV_PROTO)
extern "C++" int unsetenv (const char *);
#endif

#cmakedefine HAVE_USLEEP_PROTO 1
#if !defined(HAVE_USLEEP_PROTO)
extern "C++" int usleep (unsigned int);
#endif

#cmakedefine HAVE_INITGROUPS_PROTO 1
#if !defined(HAVE_INITGROUPS_PROTO)
#include <unistd.h>
extern "C++" int initgroups(const char *, gid_t);
#endif

#cmakedefine HAVE_MKDTEMP_PROTO 1
#if !defined(HAVE_MKDTEMP_PROTO)
extern "C++" char *mkdtemp(char *);
#endif

#cmakedefine HAVE_MKSTEMPS_PROTO 1
#if !defined(HAVE_MKSTEMPS_PROTO)
extern "C++" int mkstemps(char *, int);
#endif

#cmakedefine HAVE_MKSTEMP_PROTO 1
#if !defined(HAVE_MKSTEMP_PROTO)
extern "C++" int mkstemp(char *);
#endif

#cmakedefine HAVE_TRUNC 1
#if !defined(HAVE_TRUNC)
extern "C++" double trunc(double);
#endif

#cmakedefine HAVE_GETGROUPLIST 1
#if !defined(HAVE_GETGROUPLIST)
#include <sys/types.h> /* for gid_t */
extern "C++" int getgrouplist(const char *, gid_t , gid_t *, int *);
#endif


#endif /* KDEFAKES_H */
