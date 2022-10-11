# Try to find libjpeg-turbo library, once done this will define:
#
#  LIBJPEG_FOUND - system has libjpeg-turbo
#  LIBJPEG_INCLUDE_DIR - the libjpeg-turbo include directory
#  LIBJPEG_LIBRARIES - the libraries needed to use libjpeg-turbo
#  LIBJPEG_DEFINITIONS - compiler switches required for using libjpeg-turbo
#
# Copyright (c) 2022 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBJPEG QUIET libturbojpeg)

set(LIBJPEG_INCLUDE_DIR ${PC_LIBJPEG_INCLUDE_DIRS})
set(LIBJPEG_LIBRARIES ${PC_LIBJPEG_LIBRARIES})
set(LIBJPEG_VERSION ${PC_LIBJPEG_VERSION})
set(LIBJPEG_DEFINITIONS ${PC_LIBJPEG_CFLAGS_OTHER})

if(NOT LIBJPEG_INCLUDE_DIR OR NOT LIBJPEG_LIBRARIES)
    find_path(LIBJPEG_INCLUDE_DIR
        NAMES turbojpeg.h
        HINTS $ENV{LIBJPEGDIR}/include
    )

    find_library(LIBJPEG_LIBRARIES
        NAMES turbojpeg
        HINTS $ENV{LIBJPEGDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibJPEG
    VERSION_VAR LIBJPEG_VERSION
    REQUIRED_VARS LIBJPEG_LIBRARIES LIBJPEG_INCLUDE_DIR
)

mark_as_advanced(LIBJPEG_INCLUDE_DIR LIBJPEG_LIBRARIES)
