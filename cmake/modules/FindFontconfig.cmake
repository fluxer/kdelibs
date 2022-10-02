# Try to find Fontconfig, once done this will define:
#
#  FONTCONFIG_FOUND - system has Fontconfig
#  FONTCONFIG_INCLUDE_DIR -the Fontconfig include directory
#  FONTCONFIG_LIBRARIES - the libraries needed to use Fontconfig
#  FONTCONFIG_DEFINITIONS - compiler switches required for using Fontconfig
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_FONTCONFIG QUIET fontconfig)

set(FONTCONFIG_INCLUDE_DIR ${PC_FONTCONFIG_INCLUDE_DIRS})
set(FONTCONFIG_LIBRARIES ${PC_FONTCONFIG_LIBRARIES})
set(FONTCONFIG_VERSION ${PC_FONTCONFIG_VERSION})
set(FONTCONFIG_DEFINITIONS ${PC_FONTCONFIG_CFLAGS_OTHER})

if(NOT FONTCONFIG_INCLUDE_DIR OR NOT FONTCONFIG_LIBRARIES)
    find_path(FONTCONFIG_INCLUDE_DIR
        NAMES fontconfig/fontconfig.h
        HINTS $ENV{FONTCONFIGDIR}/include
    )

    find_library(FONTCONFIG_LIBRARIES
        NAMES fontconfig
        HINTS $ENV{FONTCONFIGDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Fontconfig
    VERSION_VAR FONTCONFIG_VERSION
    REQUIRED_VARS FONTCONFIG_LIBRARIES FONTCONFIG_INCLUDE_DIR
)

mark_as_advanced(FONTCONFIG_INCLUDE_DIR FONTCONFIG_LIBRARIES)
