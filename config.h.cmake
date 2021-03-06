/* config.h. Generated by cmake from config.h.cmake */

/* NOTE: only add something here if it is really needed by all of kdelibs.
   Otherwise please prefer adding to the relevant config-foo.h.cmake file,
   to minimize recompilations and increase modularity. */

/****************************/

#cmakedefine HAVE_STDINT_H 1
#cmakedefine HAVE_SYS_STAT_H 1
#cmakedefine HAVE_SYS_TYPES_H 1
#cmakedefine HAVE_SYS_PARAM_H 1
#cmakedefine HAVE_SYS_TIME_H 1
#cmakedefine HAVE_SYS_SELECT_H 1
#cmakedefine HAVE_SYS_MNTTAB_H 1
#cmakedefine HAVE_SYS_MNTENT_H 1
#cmakedefine HAVE_SYS_MOUNT_H 1
#cmakedefine HAVE_FSTAB_H 1
#cmakedefine HAVE_LIMITS_H 1
#cmakedefine HAVE_MNTENT_H 1
#cmakedefine HAVE_UNISTD_H 1

#cmakedefine HAVE_BACKTRACE 1
#cmakedefine HAVE_GETMNTINFO 1
#cmakedefine HAVE_FDATASYNC 1
#cmakedefine HAVE_SENDFILE 1
#cmakedefine HAVE_SETMNTENT 1
#cmakedefine HAVE_STRTOLL 1
#cmakedefine HAVE_VSNPRINTF 1
#cmakedefine HAVE_GETGROUPLIST 1
#cmakedefine HAVE_VOLMGT 1

#cmakedefine HAVE_S_ISSOCK 1

#cmakedefine TIME_WITH_SYS_TIME 1

/* Define to 1 if you have string.h */
#cmakedefine HAVE_STRING_H 1

/* define if message translations are enabled */
#cmakedefine ENABLE_NLS 1

/* Define to 1 if you have the Xtest extension */
#cmakedefine HAVE_XTEST 1

/* Define to 1 if you have the Xcursor library */
#cmakedefine HAVE_XCURSOR 1

/* Define to 1 if you have the Xfixes library */
#cmakedefine HAVE_XFIXES 1

/* Define to 1 if you have the Xscreensaver extension */
#cmakedefine HAVE_XSCREENSAVER 1

/* Define to 1 if you have the XSync extension */
#cmakedefine HAVE_XSYNC 1

/* Define to 1 if you have libintl */
#cmakedefine HAVE_LIBINTL 1

/*********************/

#ifndef HAVE_S_ISSOCK
#define HAVE_S_ISSOCK
#define S_ISSOCK(mode) (1==0)
#endif

/*
 * On HP-UX, the declaration of vsnprintf() is needed every time !
 */

#if !defined(HAVE_VSNPRINTF)
#if __STDC__
#include <stdarg.h>
#include <stdlib.h>
#else
#include <varargs.h>
#endif
#ifdef __cplusplus
extern "C"
#endif
int vsnprintf(char *str, size_t n, char const *fmt, va_list ap);
#ifdef __cplusplus
extern "C"
#endif
int snprintf(char *str, size_t n, char const *fmt, ...);
#endif

#cmakedefine GETMNTINFO_USES_STATVFS 1

/* Defined to 1 if you have a d_type member in struct dirent */
#cmakedefine HAVE_DIRENT_D_TYPE 1
