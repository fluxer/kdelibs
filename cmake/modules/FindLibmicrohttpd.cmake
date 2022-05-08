# Try to find Libmicrohttpd library, once done this will define:
#
#  LIBMICROHTTPD_FOUND - system has Libmicrohttpd
#  LIBMICROHTTPD_INCLUDE_DIR - the Libmicrohttpd include directory
#  LIBMICROHTTPD_LIBRARIES - the libraries needed to use Libmicrohttpd
#
# Copyright (c) 2022 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBMICROHTTPD QUIET libmicrohttpd)

    set(LIBMICROHTTPD_INCLUDE_DIR ${PC_LIBMICROHTTPD_INCLUDE_DIRS})
    set(LIBMICROHTTPD_LIBRARIES ${PC_LIBMICROHTTPD_LIBRARIES})
endif()

set(LIBMICROHTTPD_VERSION ${PC_LIBMICROHTTPD_VERSION})
set(LIBMICROHTTPD_DEFINITIONS ${PC_LIBMICROHTTPD_CFLAGS_OTHER})

if(NOT LIBMICROHTTPD_INCLUDE_DIR OR NOT LIBMICROHTTPD_LIBRARIES)
    find_path(LIBMICROHTTPD_INCLUDE_DIR
        NAMES microhttpd.h
        HINTS $ENV{LIBMICROHTTPDDIR}/include
    )

    find_library(LIBMICROHTTPD_LIBRARIES
        NAMES microhttpd
        HINTS $ENV{LIBMICROHTTPDDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libmicrohttpd
    VERSION_VAR LIBMICROHTTPD_VERSION
    REQUIRED_VARS LIBMICROHTTPD_LIBRARIES LIBMICROHTTPD_INCLUDE_DIR
)

mark_as_advanced(LIBMICROHTTPD_INCLUDE_DIR LIBMICROHTTPD_LIBRARIES)
