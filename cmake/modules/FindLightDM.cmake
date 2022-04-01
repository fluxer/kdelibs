# Try to find LightDM library, once done this will define:
#
#  LIGHTDM_FOUND - system has LightDM
#  LIGHTDM_INCLUDE_DIR - the LightDM include directory
#  LIGHTDM_LIBRARIES - the libraries needed to use LightDM
#  LIGHTDM_DEFINITIONS - compiler switches required for using LightDM
#
# Copyright (c) 2022 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIGHTDM QUIET liblightdm-gobject-1)

    set(LIGHTDM_INCLUDE_DIR ${PC_LIGHTDM_INCLUDE_DIRS})
    set(LIGHTDM_LIBRARIES ${PC_LIGHTDM_LIBRARIES})
endif()

set(LIGHTDM_VERSION ${PC_LIGHTDM_VERSION})
set(LIGHTDM_DEFINITIONS ${PC_LIGHTDM_CFLAGS_OTHER})

if(NOT LIGHTDM_INCLUDE_DIR OR NOT LIGHTDM_LIBRARIES)
    find_path(LIGHTDM_INCLUDE_DIR
        NAMES lightdm-gobject-1/lightdm.h
        HINTS $ENV{LIGHTDMDIR}/include
    )

    find_library(LIGHTDM_LIBRARIES
        NAMES lightdm-gobject-1
        HINTS $ENV{LIGHTDMDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LightDM
    VERSION_VAR LIGHTDM_VERSION
    REQUIRED_VARS LIGHTDM_LIBRARIES LIGHTDM_INCLUDE_DIR
)

mark_as_advanced(LIGHTDM_INCLUDE_DIR LIGHTDM_LIBRARIES)
