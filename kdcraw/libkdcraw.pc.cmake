prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${KDE4_LIBEXEC_INSTALL_DIR}
libdir=${KDE4_LIB_INSTALL_DIR}
includedir=${KDE4_INCLUDE_INSTALL_DIR}

Name: libkdcraw
Description: A C++ wrapper around LibRaw library to decode RAW pictures. This library is used by digiKam and kipi-plugins.
URL: http://www.digikam.org/sharedlibs
Requires:
Version: ${DCRAW_LIB_VERSION_STRING}
Libs: -L${KDE4_LIB_INSTALL_DIR} -lkdcraw
Cflags: -I${KDE4_INCLUDE_INSTALL_DIR}
