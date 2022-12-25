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
#cmakedefine HAVE_ARC4RANDOM_UNIFORM 1
#cmakedefine HAVE_SENDFILE 1
#cmakedefine HAVE_SETMNTENT 1
#cmakedefine HAVE_STRTOLL 1
#cmakedefine HAVE_STRMODE 1

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

/* Define to 1 if you have DBusMenuQt */
#cmakedefine HAVE_DBUSMENUQT 1

/* Define to 1 if you have libarchive */
#cmakedefine HAVE_LIBARCHIVE 1

/* Define to 1 if you have BZip2 */
#cmakedefine HAVE_BZIP2 1

/* Define to 1 if you have XZ Utils */
#cmakedefine HAVE_LIBLZMA 1

/* Define to 1 if you have Avahi */
#cmakedefine HAVE_AVAHI 1

/* Define to 1 if you have Exiv2 */
#cmakedefine HAVE_EXIV2 1

/* Define to 1 if you have MPV */
#cmakedefine HAVE_MPV 1

/* Define to 1 if you have OpenSSL */
#cmakedefine HAVE_OPENSSL 1

/* Define to 1 if getmntinfo() uses statvfs struct */
#cmakedefine GETMNTINFO_USES_STATVFS 1

/* Defined to 1 if you have a d_type member in struct dirent */
#cmakedefine HAVE_DIRENT_D_TYPE 1

/* Needed for the kdecore and solid tests */
#cmakedefine ENABLE_TESTING 1
