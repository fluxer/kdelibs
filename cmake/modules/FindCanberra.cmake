# - Try to find Canberra library
#
# Once done this will define
#
#  CANBERRA_FOUND - system has Canberra
#  CANBERRA_INCLUDE_DIRS - the Canberra include directory
#  CANBERRA_LIBRARIES - the libraries needed to use Canberra
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_CANBERRA QUIET libcanberra)

    set(CANBERRA_INCLUDE_DIRS ${PC_CANBERRA_INCLUDE_DIRS})
    set(CANBERRA_LIBRARIES ${PC_CANBERRA_LIBRARIES})
endif()

set(CANBERRA_VERSION ${PC_CANBERRA_VERSION})

if(NOT CANBERRA_INCLUDE_DIRS OR NOT CANBERRA_LIBRARIES)
    find_path(CANBERRA_INCLUDE_DIRS
        NAMES canberra.h
        HINTS $ENV{CANBERRADIR}/include
    )

    find_library(CANBERRA_LIBRARIES
        NAMES canberra
        HINTS $ENV{CANBERRADIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Canberra
    VERSION_VAR CANBERRA_VERSION
    REQUIRED_VARS CANBERRA_LIBRARIES CANBERRA_INCLUDE_DIRS
)

mark_as_advanced(CANBERRA_INCLUDE_DIRS CANBERRA_LIBRARIES)
