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

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBDEFLATE QUIET libdeflate)

set(LIBDEFLATE_INCLUDES ${PC_LIBDEFLATE_INCLUDE_DIRS})
set(LIBDEFLATE_LIBRARIES ${PC_LIBDEFLATE_LIBRARIES})
set(LIBDEFLATE_VERSION ${PC_LIBDEFLATE_VERSION})

if(NOT LIBDEFLATE_INCLUDES OR NOT LIBDEFLATE_LIBRARIES)
    find_path(LIBDEFLATE_INCLUDES
        NAMES libdeflate.h
        HINTS $ENV{LIBDEFLATEDIR}/include
    )

    find_library(LIBDEFLATE_LIBRARIES
        NAMES deflate
        HINTS $ENV{LIBDEFLATEDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibDeflate
    VERSION_VAR LIBDEFLATE_VERSION
    REQUIRED_VARS LIBDEFLATE_LIBRARIES LIBDEFLATE_INCLUDES
)

mark_as_advanced(LIBDEFLATE_INCLUDES LIBDEFLATE_LIBRARIES)
