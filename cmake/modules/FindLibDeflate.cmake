# Try to find libdeflate, once done this will define:
#
#  LIBDEFLATE_FOUND - system has libdeflate
#  LIBDEFLATE_INCLUDES - the libdeflate include directory
#  LIBDEFLATE_LIBRARIES - the libraries needed to use libdeflate
#
# Copyright (c) 2022 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# libdeflate does not provide pkg-config files

find_package(PkgConfig REQUIRED)
include(FindPackageHandleStandardArgs)

find_path(LIBDEFLATE_INCLUDES
    NAMES libdeflate.h
    HINTS $ENV{LIBDEFLATEDIR}/include
)

find_library(LIBDEFLATE_LIBRARIES
    NAMES deflate
    HINTS $ENV{LIBDEFLATEDIR}/lib
)

find_package_handle_standard_args(LibDeflate
    REQUIRED_VARS LIBDEFLATE_LIBRARIES LIBDEFLATE_INCLUDES
)

mark_as_advanced(LIBDEFLATE_INCLUDES LIBDEFLATE_LIBRARIES)
