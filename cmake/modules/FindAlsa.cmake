# - Try to find ALSA
#
# Once done this will define
#
#  ALSA_FOUND - system has ALSA
#  ALSA_INCLUDES - the ALSA include directory
#  ALSA_LIBRARIES - The libraries needed to use ALSA
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if(ALSA_INCLUDES AND ALSA_LIBRARIES)
    set(ALSA_FIND_QUIETLY TRUE)
endif()

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_ALSA QUIET alsa)

    set(ALSA_INCLUDES ${PC_ALSA_INCLUDE_DIRS})
    set(ALSA_LIBRARIES ${PC_ALSA_LIBRARIES})
endif()

if(NOT ALSA_INCLUDES OR NOT ALSA_LIBRARIES)
    find_path(ALSA_INCLUDES
        NAMES alsa/version.h
        HINTS $ENV{ALSADIR}/include
    )

    find_library(ALSA_LIBRARIES
        NAMES asound2 asound
        HINTS $ENV{ALSADIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Alsa
    VERSION_VAR PC_ALSA_VERSION
    REQUIRED_VARS ALSA_LIBRARIES ALSA_INCLUDES
)

mark_as_advanced(ALSA_INCLUDES ALSA_LIBRARIES)
