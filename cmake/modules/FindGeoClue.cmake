# Try to find GeoClue, once done this will define:
#
#  GEOCLUE_FOUND - system has GeoClue
#  GEOCLUE_INCLUDES - the GeoClue include directory
#  GEOCLUE_LIBRARIES - the libraries needed to use GeoClue
#
# Copyright (c) 2022 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_GEOCLUE QUIET libgeoclue-2.0)

    set(GEOCLUE_INCLUDES ${PC_GEOCLUE_INCLUDE_DIRS})
    set(GEOCLUE_LIBRARIES ${PC_GEOCLUE_LIBRARIES})
endif()

set(GEOCLUE_VERSION ${PC_GEOCLUE_VERSION})

if(NOT GEOCLUE_INCLUDES OR NOT GEOCLUE_LIBRARIES)
    find_path(GEOCLUE_INCLUDES
        NAMES libgeoclue-2.0/gclue-simple.h
        HINTS $ENV{GEOCLUEDIR}/include
    )

    find_library(GEOCLUE_LIBRARIES
        NAMES geoclue-2
        HINTS $ENV{GEOCLUEDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GeoClue
    VERSION_VAR GEOCLUE_VERSION
    REQUIRED_VARS GEOCLUE_LIBRARIES GEOCLUE_INCLUDES
)

mark_as_advanced(GEOCLUE_INCLUDES GEOCLUE_LIBRARIES)
