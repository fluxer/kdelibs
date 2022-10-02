# Try to find the LibLZMA, once done this will define:
#
#  LIBLZMA_FOUND - system has LibLZMA
#  LIBLZMA_INCLUDE_DIRS - the LibLZMA include directory
#  LIBLZMA_LIBRARIES - the libraries needed to use LibLZMA
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBLZMA QUIET liblzma)

set(LIBLZMA_INCLUDE_DIR ${PC_LIBLZMA_INCLUDE_DIRS})
set(LIBLZMA_LIBRARIES ${PC_LIBLZMA_LIBRARIES})
set(LIBLZMA_VERSION ${PC_LIBLZMA_VERSION})

if(NOT LIBLZMA_INCLUDE_DIR OR NOT LIBLZMA_LIBRARIES)
    find_path(LIBLZMA_INCLUDE_DIR
        NAMES lzma.h
        HINTS $ENV{LIBLZMADIR}/include
    )

    find_library(LIBLZMA_LIBRARIES
        NAMES lzma
        HINTS $ENV{LIBLZMADIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibLZMA
    VERSION_VAR LIBLZMA_VERSION
    REQUIRED_VARS LIBLZMA_LIBRARIES LIBLZMA_INCLUDE_DIR
)

mark_as_advanced(LIBLZMA_INCLUDE_DIR LIBLZMA_LIBRARIES)
