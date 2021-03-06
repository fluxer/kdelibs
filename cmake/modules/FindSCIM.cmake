# Try to find the SCIM, once done this will define:
#
#  SCIM_FOUND - system has SCIM
#  SCIM_INCLUDE_DIR - the SCIM include directory
#  SCIM_LIBRARIES - the libraries needed to use SCIM
#  SCIM_DEFINITIONS - compiler switches required for using SCIM
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_SCIM QUIET scim)

    set(SCIM_INCLUDE_DIR ${PC_SCIM_INCLUDE_DIRS})
    set(SCIM_LIBRARIES ${PC_SCIM_LIBRARIES})
endif()

set(SCIM_VERSION ${PC_SCIM_VERSION})
set(SCIM_DEFINITIONS ${PC_SCIM_CFLAGS_OTHER})

if(NOT SCIM_INCLUDE_DIR OR NOT SCIM_LIBRARIES)
    find_path(SCIM_INCLUDE_DIR
        NAMES scim.h
        HINTS $ENV{SCIMDIR}/include
    )

    find_library(SCIM_LIBRARIES
        NAMES scim scim-1.0
        HINTS $ENV{SCIMDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SCIM
    VERSION_VAR SCIM_VERSION
    REQUIRED_VARS SCIM_LIBRARIES SCIM_INCLUDE_DIR
)

mark_as_advanced(SCIM_INCLUDE_DIR SCIM_LIBRARIES)
