# - Try to find the FLAC++ library
#
# Once done this will define
#
#  FLAC_FOUND - system has FLAC++
#  FLAC_INCLUDES - the FLAC++ include directory
#  FLAC_LIBRARIES - Link these to use FLAC++
#  FLAC_DEFINITIONS - Compiler switches required for using FLAC++

# Copyright (c) 2019, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    pkg_check_modules(PC_FLAC QUIET flac++)
    set(FLAC_DEFINITIONS ${PC_FLAC_CFLAGS_OTHER})
    set(FLAC_VERSION ${PC_FLAC_VERSION})
endif()


find_path(FLAC_INCLUDES
    NAMES
    FLAC++/metadata.h
    HINTS
    ${PC_FLAC_INCLUDEDIR}
    ${PC_FLAC_INCLUDESS}
)

find_library(FLAC_LIBRARIES
    NAMES
    FLAC++
    HINTS
    ${PC_FLAC_LIBDIR}
    ${PC_FLAC_LIBRARIES_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FLAC++
    REQUIRED_VARS  FLAC_LIBRARIES FLAC_INCLUDES
    VERSION_VAR  FLAC_VERSION
)

if(FLAC_LIBRARIES AND FLAC_INCLUDES)
    set(FLAC_FOUND TRUE)
endif()

mark_as_advanced(FLAC_INCLUDES FLAC_LIBRARIES)

