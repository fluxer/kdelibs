# - Try to find the raw1394
#
# Once done this will define
#
#  RAW1394_FOUND - system has raw1394
#  RAW1394_INCLUDE_DIR - the raw1394 include directory
#  RAW1394_LIBRARIES - the libraries needed to use raw1394
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_RAW1394 QUIET libraw1394)

    set(RAW1394_INCLUDE_DIR ${PC_RAW1394_INCLUDE_DIRS})
    set(RAW1394_LIBRARIES ${PC_RAW1394_LIBRARIES})
endif()

set(RAW1394_VERSION ${PC_RAW1394_VERSION})

if(NOT RAW1394_INCLUDE_DIR OR NOT RAW1394_LIBRARIES)
    find_path(RAW1394_INCLUDE_DIR
        NAMES libraw1394/raw1394.h
        HINTS $ENV{RAW1394DIR}/include
    )

    find_library(RAW1394_LIBRARIES
        NAMES raw1394
        HINTS $ENV{RAW1394DIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RAW1394
    VERSION_VAR RAW1394_VERSION
    REQUIRED_VARS RAW1394_LIBRARIES RAW1394_INCLUDE_DIR
)

mark_as_advanced(RAW1394_INCLUDE_DIR RAW1394_LIBRARIES)
