# Try to find OpenEXR, once done this will define:
#
#  OPENEXR_FOUND - system has OpenEXR
#  OPENEXR_INCLUDE_DIR - the OpenEXR include directory
#  OPENEXR_LIBRARIES - the libraries needed to use OpenEXR
#  OPENEXR_DEFINITIONS - compiler switches required for using OpenEXR
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_OPENEXR QUIET OpenEXR)

    set(OPENEXR_INCLUDE_DIR ${PC_OPENEXR_INCLUDE_DIRS})
    set(OPENEXR_LIBRARIES ${PC_OPENEXR_LIBRARIES})
endif()

set(OPENEXR_VERSION ${PC_OPENEXR_VERSION})
set(OPENEXR_DEFINITIONS ${PC_OPENEXR_CFLAGS_OTHER})

if(NOT OPENEXR_INCLUDE_DIR OR NOT OPENEXR_LIBRARIES)
    find_path(OPENEXR_INCLUDE_DIR
        NAMES ImfRgbaFile.h
        PATH_SUFFIXES OpenEXR
        HINTS $ENV{OPENEXRDIR}/include
    )

    find_library(OPENEXR_LIBRARIES
        NAMES OpenEXR
        HINTS $ENV{OPENEXRDIR}/lib
    )

    # fallback for versions older than 3.0
    if(NOT OPENEXR_LIBRARIES)
        set(OPENEXR_NAMES Imath IlmImf Half Iex IlmThread)
        foreach(lib ${OPENEXR_NAMES})
            find_library(OPENEXR_${lib}_PATH
                NAMES ${lib}
                HINTS $ENV{OPENEXRDIR}/lib
            )
            set(OPENEXR_LIBRARIES ${OPENEXR_LIBRARIES} ${OPENEXR_${lib}_PATH})
        endforeach()
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenEXR
    VERSION_VAR OPENEXR_VERSION
    REQUIRED_VARS OPENEXR_LIBRARIES OPENEXR_INCLUDE_DIR
)

mark_as_advanced(OPENEXR_INCLUDE_DIR OPENEXR_LIBRARIES)
