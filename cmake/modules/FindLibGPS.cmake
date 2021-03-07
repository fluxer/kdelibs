# Try to find GPS library, once done this will define:
#
# LIBGPS_FOUND - system has GPS library
# LIBGPS_INCLUDE_DIR - the GPS library include directory
# LIBGPS_LIBRARIES - the libraries needed to use GPS library
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBGPS QUIET libgps)

    set(LIBGPS_INCLUDE_DIR ${PC_LIBGPS_INCLUDE_DIRS})
    set(LIBGPS_LIBRARIES ${PC_LIBGPS_LIBRARIES})
endif()

set(LIBGPS_VERSION ${PC_LIBGPS_VERSION})

if(NOT LIBGPS_INCLUDE_DIR OR NOT LIBGPS_LIBRARIES)
    find_path(LIBGPS_INCLUDE_DIR
        NAMES gps.h
        HINTS $ENV{LIBGPSDIR}/include
    )

    find_library(LIBGPS_LIBRARIES
        NAMES gps
        HINTS $ENV{LIBGPSDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibGPS
    VERSION_VAR LIBGPS_VERSION
    REQUIRED_VARS LIBGPS_LIBRARIES LIBGPS_INCLUDE_DIR
)

mark_as_advanced(LIBGPS_INCLUDE_DIR LIBGPS_LIBRARIES)
