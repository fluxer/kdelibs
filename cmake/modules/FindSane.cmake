# Try to find SANE, once done this will define:
#
#  SANE_FOUND - system has SANE libs
#  SANE_INCLUDE_DIR - the SANE include directory
#  SANE_LIBRARIES - The libraries needed to use SANE
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(SANE_INCLUDE_DIR
    NAMES sane/sane.h
    HINTS $ENV{SANEDIR}/include
)

find_library(SANE_LIBRARY
    NAMES sane
    PATH_SUFFIXES sane
    HINTS $ENV{SANEDIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sane
    REQUIRED_VARS SANE_LIBRARY SANE_INCLUDE_DIR
)

mark_as_advanced(SANE_INCLUDE_DIR SANE_LIBRARY)
