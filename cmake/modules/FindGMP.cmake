# Try to find GMP, once done this will define:
#
#  GMP_FOUND - system has GMP
#  GMP_INCLUDE_DIR - the GMP include directory
#  GMP_LIBRARIES - the libraries needed to use GMP
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(GMP_INCLUDE_DIR
    NAMES gmp.h
    HINTS $ENV{GMPDIR}/include
)

find_library(GMP_LIBRARIES
    NAMES gmp
    HINTS $ENV{GMPDIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP
    REQUIRED_VARS GMP_LIBRARIES GMP_INCLUDE_DIR
)

mark_as_advanced(GMP_INCLUDE_DIR GMP_LIBRARIES)
