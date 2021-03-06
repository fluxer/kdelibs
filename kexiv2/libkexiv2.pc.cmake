prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${KDE4_LIBEXEC_INSTALL_DIR}
libdir=${KDE4_LIB_INSTALL_DIR}
includedir=${KDE4_INCLUDE_INSTALL_DIR}

Name: libkexiv2
Description: A C++ library to manipulate EXIF/IPTC/XMP metadata using Exiv2 library. This library is used by digiKam and kipi-plugins.
URL: http://www.digikam.org
Requires:
Version: ${KEXIV2_LIB_VERSION_STRING}
Libs: -L${KDE4_LIB_INSTALL_DIR} -lkexiv2
Cflags: -I${KDE4_INCLUDE_INSTALL_DIR}
