/* config.h. Generated by cmake from config.h.cmake */

/* NOTE: only add something here if it is really needed by all of kdelibs.
   Otherwise please prefer adding to the relevant config-foo.h.cmake file,
   to minimize recompilations and increase modularity. */

/****************************/

#cmakedefine HAVE_SYS_PARAM_H 1
#cmakedefine HAVE_SYS_MNTTAB_H 1
#cmakedefine HAVE_SYS_MNTENT_H 1
#cmakedefine HAVE_SYS_MOUNT_H 1
#cmakedefine HAVE_FSTAB_H 1
#cmakedefine HAVE_MNTENT_H 1
#cmakedefine HAVE_PATHS_H 1

#cmakedefine HAVE_BACKTRACE 1
#cmakedefine HAVE_GETMNTINFO 1
#cmakedefine HAVE_FDATASYNC 1
#cmakedefine HAVE_SENDFILE 1
#cmakedefine HAVE_SETMNTENT 1
#cmakedefine HAVE_STRTOLL 1
#cmakedefine HAVE_GETGROUPLIST 1
#cmakedefine HAVE_TTYNAME_R 1

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

/* Define to 1 if getmntinfo() uses statvfs struct */
#cmakedefine GETMNTINFO_USES_STATVFS 1

/* Defined to 1 if you have a d_type member in struct dirent */
#cmakedefine HAVE_DIRENT_D_TYPE 1

/* Needed for the kdecore and solid tests in release mode */
#cmakedefine ENABLE_TESTING 1
