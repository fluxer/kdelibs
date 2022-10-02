# Try to find kmod library, once done this will define:
#
#  KMOD_FOUND - system has kmod
#  KMOD_INCLUDE_DIR - the kmod include directory
#  KMOD_LIBRARIES - the libraries needed to use kmod
#
# Copyright (c) 2022 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_KMOD QUIET libkmod)

set(KMOD_INCLUDE_DIR ${PC_KMOD_INCLUDE_DIRS})
set(KMOD_LIBRARIES ${PC_KMOD_LIBRARIES})
set(KMOD_VERSION ${PC_KMOD_VERSION})

if(NOT KMOD_INCLUDE_DIR OR NOT KMOD_LIBRARIES)
    find_path(KMOD_INCLUDE_DIR
        NAMES libkmod.h
        HINTS $ENV{KMODDIR}/include
    )

    find_library(KMOD_LIBRARIES
        NAMES kmod
        HINTS $ENV{KMODDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Kmod
    VERSION_VAR KMOD_VERSION
    REQUIRED_VARS KMOD_LIBRARIES KMOD_INCLUDE_DIR
)

mark_as_advanced(KMOD_INCLUDE_DIR KMOD_LIBRARIES)
