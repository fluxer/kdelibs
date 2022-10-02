# Try to find Poppler library, once done this will define:
#
#  POPPLER_FOUND - system has Poppler
#  POPPLER_INCLUDE_DIR - the Poppler include directory
#  POPPLER_LIBRARIES - the libraries needed to use Poppler
#
# Copyright (c) 2022 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_POPPLER QUIET poppler-cpp)

set(POPPLER_INCLUDE_DIR ${PC_POPPLER_INCLUDE_DIRS})
set(POPPLER_LIBRARIES ${PC_POPPLER_LIBRARIES})
set(POPPLER_VERSION ${PC_POPPLER_VERSION})

if(NOT POPPLER_INCLUDE_DIR OR NOT POPPLER_LIBRARIES)
    find_path(POPPLER_INCLUDE_DIR
        NAMES poppler/cpp/poppler-document.h
        HINTS $ENV{POPPLERDIR}/include
    )

    find_library(POPPLER_LIBRARIES
        NAMES poppler-cpp
        HINTS $ENV{POPPLERDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Poppler
    VERSION_VAR POPPLER_VERSION
    REQUIRED_VARS POPPLER_LIBRARIES POPPLER_INCLUDE_DIR
)

mark_as_advanced(POPPLER_INCLUDE_DIR POPPLER_LIBRARIES)
