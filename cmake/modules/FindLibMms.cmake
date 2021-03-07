# Try to find the libmms library, once done this will define:
#
#  LIBMMS_FOUND - system has libmms
#  LIBMMS_INCLUDE_DIR - the libmms include directory
#  LIBMMS_LIBRARIES - the libraries needed to use libmms
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBMMS QUIET libmms)

    set(LIBMMS_INCLUDE_DIR ${PC_LIBMMS_INCLUDE_DIRS})
    set(LIBMMS_LIBRARIES ${PC_LIBMMS_LIBRARIES})
endif()

set(LIBMMS_VERSION ${PC_LIBMMS_VERSION})

if(NOT LIBMMS_INCLUDE_DIR OR NOT LIBMMS_LIBRARIES)
    find_path(LIBMMS_INCLUDE_DIR
        NAMES mmsx.h
        PATH_SUFFIXES libmms
        HINTS $ENV{LIBMMSDIR}/include
    )

    find_library(LIBMMS_LIBRARIES
        NAMES mms
        HINTS $ENV{LIBMMSDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibMms
    VERSION_VAR LIBMMS_VERSION
    REQUIRED_VARS LIBMMS_LIBRARIES LIBMMS_INCLUDE_DIR
)

mark_as_advanced(LIBMMS_INCLUDE_DIR LIBMMS_LIBRARIES)
