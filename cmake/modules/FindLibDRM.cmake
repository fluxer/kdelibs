# Try to find libdrm library, once done this will define:
#
#  LIBDRM_FOUND - system has libdrm
#  LIBDRM_INCLUDE_DIR - the libdrm include directory
#  LIBDRM_LIBRARIES - the libraries needed to use libdrm
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBDRM QUIET libdrm)

set(LIBDRM_INCLUDE_DIR ${PC_LIBDRM_INCLUDE_DIRS})
set(LIBDRM_LIBRARIES ${PC_LIBDRM_LIBRARIES})
set(LIBDRM_VERSION ${PC_LIBDRM_VERSION})

if(NOT LIBDRM_INCLUDE_DIR OR NOT LIBDRM_LIBRARIES)
    find_path(LIBDRM_INCLUDE_DIR
        NAMES libdrm/drm.h
        HINTS $ENV{LIBDRMDIR}/include
    )

    find_library(LIBDRM_LIBRARIES
        NAMES drm
        HINTS $ENV{LIBDRMDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibDRM
    VERSION_VAR LIBDRM_VERSION
    REQUIRED_VARS LIBDRM_LIBRARIES LIBDRM_INCLUDE_DIR
)

mark_as_advanced(LIBDRM_INCLUDE_DIR LIBDRM_LIBRARIES)
