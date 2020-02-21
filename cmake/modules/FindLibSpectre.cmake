# - Try to find the libspectre PS library
#
# Once done this will define
#
#  LIBSPECTRE_FOUND - system has libspectre
#  LIBSPECTRE_INCLUDE_DIR - the libspectre include directory
#  LIBSPECTRE_LIBRARY - Link this to use libspectre
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBSPECTRE QUIET libspectre)

    set(LIBSPECTRE_INCLUDE_DIR ${PC_LIBSPECTRE_INCLUDE_DIRS})
    set(LIBSPECTRE_LIBRARY ${PC_LIBSPECTRE_LIBRARIES})
endif()

set(LIBSPECTRE_VERSION ${PC_LIBSPECTRE_VERSION})

if(NOT LIBSPECTRE_INCLUDE_DIR OR NOT LIBSPECTRE_LIBRARY)
    find_path(LIBSPECTRE_INCLUDE_DIR
        NAMES spectre.h
        PATH_SUFFIXES libspectre
        HINTS $ENV{LIBSPECTREDIR}/include
    )

    find_library(LIBSPECTRE_LIBRARY
        NAMES spectre
        HINTS $ENV{LIBSPECTREDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibSpectre
    VERSION_VAR LIBSPECTRE_VERSION
    REQUIRED_VARS LIBSPECTRE_LIBRARY LIBSPECTRE_INCLUDE_DIR
)

mark_as_advanced(LIBSPECTRE_INCLUDE_DIR LIBSPECTRE_LIBRARY)
