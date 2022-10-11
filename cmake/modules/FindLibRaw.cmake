# Try to find LibRaw library, once done this will define:
#
#  LIBRAW_FOUND - system has LibRaw
#  LIBRAW_INCLUDE_DIR - the LibRaw include directory
#  LIBRAW_LIBRARIES - the libraries needed to use LibRaw
#  LIBRAW_DEFINITIONS - compiler switches required for using LibRaw
#
# Copyright (c) 2022 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBRAW QUIET libraw_r)

set(LIBRAW_INCLUDE_DIR ${PC_LIBRAW_INCLUDE_DIRS})
set(LIBRAW_LIBRARIES ${PC_LIBRAW_LIBRARIES})
set(LIBRAW_VERSION ${PC_LIBRAW_VERSION})
set(LIBRAW_DEFINITIONS ${PC_LIBRAW_CFLAGS_OTHER})

if(NOT LIBRAW_INCLUDE_DIR OR NOT LIBRAW_LIBRARIES)
    find_path(LIBRAW_INCLUDE_DIR
        NAMES libraw/libraw.h
        HINTS $ENV{LIBRAWDIR}/include
    )

    find_library(LIBRAW_LIBRARIES
        NAMES raw_r
        HINTS $ENV{LIBRAWDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibRaw
    VERSION_VAR LIBRAW_VERSION
    REQUIRED_VARS LIBRAW_LIBRARIES LIBRAW_INCLUDE_DIR
)

mark_as_advanced(LIBRAW_INCLUDE_DIR LIBRAW_LIBRARIES)
