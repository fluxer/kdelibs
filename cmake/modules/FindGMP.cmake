# - Try to find GMP
# Once done this will define
#
#  GMP_FOUND - system has GMP
#  GMP_INCLUDE_DIR - the GMP include directory
#  GMP_LIBRARIES - The libraries needed to use GMP
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if(GMP_INCLUDE_DIR AND GMP_LIBRARIES)
    set(GMP_FIND_QUIETLY TRUE)
endif()

find_path(GMP_INCLUDE_DIR
    NAMES
    gmp.h
    HINTS
    $ENV{GMPDIR}/include
    ${PC_GMP_INCLUDEDIR}
    ${INCLUDE_INSTALL_DIR}
)

find_library(GMP_LIBRARIES
    gmp
    HINTS
    $ENV{GMPDIR}/lib
    ${PC_GMP_LIBDIR}
    ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP
    REQUIRED_VARS GMP_LIBRARIES GMP_INCLUDE_DIR
)

mark_as_advanced(GMP_INCLUDE_DIR GMP_LIBRARIES)
