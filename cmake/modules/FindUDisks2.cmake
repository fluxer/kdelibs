# - Try to find UDisks2
#
# Once done this will define
#
#  UDISKS2_FOUND - system has UDisks2
#  UDISKS2_INCLUDE_DIR - the UDisks2 include directory
#  UDISKS2_LIBRARIES - The libraries needed to use UDisks2
#
# Copyright (c) 2014-2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(UDISKS2_INCLUDE_DIR AND UDISKS2_LIBRARIES)
    set(UDISKS2_FIND_QUIETLY TRUE)
endif()

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_UDISKS2 QUIET udisks2)

    set(UDISKS2_INCLUDE_DIR ${PC_UDISKS2_INCLUDE_DIRS})
    set(UDISKS2_LIBRARIES ${PC_UDISKS2_LIBRARIES})
endif()

set(UDISKS2_VERSION ${PC_UDISKS2_VERSION})

if(NOT UDISKS2_INCLUDE_DIR OR NOT UDISKS2_LIBRARIES)
    find_path(UDISKS2_INCLUDE_DIR
        NAMES udisks2/udisks/udisks.h
        HINTS $ENV{UDISKS2DIR}/include
    )

    find_library(UDISKS2_LIBRARIES
        NAMES udisks2
        HINTS $ENV{UDISKS2DIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UDisks2
    VERSION_VAR UDISKS2_VERSION
    REQUIRED_VARS UDISKS2_LIBRARIES UDISKS2_INCLUDE_DIR
)

mark_as_advanced(UDISKS2_INCLUDE_DIR UDISKS2_LIBRARIES)
