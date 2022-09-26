# Try to find speech-dispatcher, once done this will define:
#
#  SPEECHD_FOUND - system has speech-dispatcher
#  SPEECHD_INCLUDE_DIR - the speech-dispatcher include directory
#  SPEECHD_LIBRARIES - the libraries needed to use speech-dispatcher
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_SPEECHD QUIET speech-dispatcher)

    set(SPEECHD_INCLUDE_DIR ${PC_SPEECHD_INCLUDE_DIRS})
    set(SPEECHD_LIBRARIES ${PC_SPEECHD_LIBRARIES})
endif()

set(SPEECHD_VERSION ${PC_SPEECHD_VERSION})

if(NOT SPEECHD_INCLUDE_DIR OR NOT SPEECHD_LIBRARIES)
    find_path(SPEECHD_INCLUDE_DIR
        NAMES libspeechd.h
        PATH_SUFFIXES speech-dispatcher
        HINTS $ENV{SPEECHDDIR}/include
    )

    find_library(SPEECHD_LIBRARIES
        NAMES speechd
        HINTS $ENV{SPEECHDDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Speechd
    VERSION_VAR SPEECHD_VERSION
    REQUIRED_VARS SPEECHD_INCLUDE_DIR SPEECHD_LIBRARIES
)

mark_as_advanced(SPEECHD_INCLUDE_DIR SPEECHD_LIBRARIES)
