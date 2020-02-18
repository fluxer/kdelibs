# - Try to find the FLAC++ library
#
# Once done this will define
#
#  FLAC_FOUND - system has FLAC++
#  FLAC_INCLUDES - the FLAC++ include directory
#  FLAC_LIBRARIES - Link these to use FLAC++
#  FLAC_DEFINITIONS - Compiler switches required for using FLAC++
#
# Copyright (c) 2019-2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    find_package(PkgConfig)
    pkg_check_modules(PC_FLAC QUIET flac++)

    set(FLAC_INCLUDES ${PC_FLAC_INCLUDE_DIRS})
    set(FLAC_LIBRARIES ${PC_FLAC_LIBRARIES})
endif()

set(FLAC_VERSION ${PC_FLAC_VERSION})
set(FLAC_DEFINITIONS ${PC_FLAC_CFLAGS_OTHER})

if(NOT FLAC_LIBRARIES OR NOT FLAC_INCLUDES)
    find_path(FLAC_INCLUDES
        NAMES FLAC++/metadata.h
        HINTS $ENV{FLACPPDIR}/include
    )

    find_library(FLAC_LIBRARIES
        NAMES FLAC++
        HINTS $ENV{FLACPPDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FLAC++
    VERSION_VAR FLAC_VERSION
    REQUIRED_VARS FLAC_INCLUDES FLAC_LIBRARIES
)

mark_as_advanced(FLAC_INCLUDES FLAC_LIBRARIES)
