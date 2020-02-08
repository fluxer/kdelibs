# - Try to find MPFR
# Once done this will define
#
#  MPFR_FOUND - system has MPFR
#  MPFR_INCLUDE_DIR - the MPFR include directory
#  MPFR_LIBRARIES - The libraries needed to use MPFR
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if(MPFR_INCLUDE_DIR AND MPFR_LIBRARIES)
    set(MPFR_FIND_QUIETLY TRUE)
endif()

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_MPFR QUIET mpfr)
endif()

find_path(MPFR_INCLUDE_DIR
    NAMES
    mpfr.h
    HINTS
    $ENV{MPFRDIR}/include
    ${PC_MPFR_INCLUDEDIR}
    ${INCLUDE_INSTALL_DIR}
)

find_library(MPFR_LIBRARIES
    mpfr
    HINTS
    $ENV{MPFRDIR}/lib
    ${PC_MPFR_LIBDIR}
    ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MPFR
    VERSION_VAR PC_MPFR_VERSION
    REQUIRED_VARS MPFR_LIBRARIES MPFR_INCLUDE_DIR
)

mark_as_advanced(MPFR_INCLUDE_DIR MPFR_LIBRARIES)
