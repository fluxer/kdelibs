# - Try to find ALSA
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
endif()

find_path(ALSA_INCLUDES
    NAMES
    alsa/version.h
    HINTS
    $ENV{ALSADIR}/include
    ${PC_ALSA_INCLUDEDIR}
    ${INCLUDE_INSTALL_DIR}
)

find_library(ALSA_LIBRARIES
    asound2 asound
    HINTS
    $ENV{ALSADIR}/lib
    ${PC_ALSA_LIBDIR}
    ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Alsa
    VERSION_VAR PC_ALSA_VERSION
    REQUIRED_VARS ALSA_LIBRARIES ALSA_INCLUDES
)

mark_as_advanced(ALSA_INCLUDES ALSA_LIBRARIES)
