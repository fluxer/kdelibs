# Try to find MPFR, once done this will define:
#
#  MPFR_FOUND - system has MPFR
#  MPFR_INCLUDE_DIR - the MPFR include directory
#  MPFR_LIBRARIES - the libraries needed to use MPFR
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_MPFR QUIET mpfr)

set(MPFR_INCLUDE_DIR ${PC_MPFR_INCLUDE_DIRS})
set(MPFR_LIBRARIES ${PC_MPFR_LIBRARIES})
set(MPFR_VERSION ${PC_MPFR_VERSION})

if(NOT MPFR_INCLUDE_DIR OR NOT MPFR_LIBRARIES)
    find_path(MPFR_INCLUDE_DIR
        NAMES mpfr.h
        HINTS $ENV{MPFRDIR}/include
    )

    find_library(MPFR_LIBRARIES
        NAMES mpfr
        HINTS $ENV{MPFRDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MPFR
    VERSION_VAR MPFR_VERSION
    REQUIRED_VARS MPFR_LIBRARIES MPFR_INCLUDE_DIR
)

mark_as_advanced(MPFR_INCLUDE_DIR MPFR_LIBRARIES)
