# - Try to find Strigi
#
# Once done this will define
#
#  STRIGI_FOUND - system has Strigi
#  STRIGI_INCLUDE_DIR - the Strigi include directory
#  STRIGI_STREAMS_LIBRARY - the libraries needed to use Strigi streams
#  STRIGI_STREAMANALYZER_LIBRARY - the libraries needed to use Strigi stream analyzer
#
# Copyright (c) 2021, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBSTREAMS QUIET libstreams)
    pkg_check_modules(PC_LIBSTREAMANALYZER QUIET libstreamanalyzer)

    set(STRIGI_INCLUDE_DIR ${PC_LIBSTREAMS_INCLUDE_DIRS})
    set(STRIGI_STREAMS_LIBRARY ${PC_LIBSTREAMS_LIBRARIES})
    set(STRIGI_STREAMANALYZER_LIBRARY ${PC_LIBSTREAMANALYZER_LIBRARIES})
endif()

set(STRIGI_VERSION ${PC_LIBSTREAMS_VERSION})

if(NOT STRIGI_INCLUDE_DIR OR NOT STRIGI_STREAMS_LIBRARY OR NOT STRIGI_STREAMANALYZER_LIBRARY)
    find_path(STRIGI_INCLUDE_DIR
        NAMES strigi/strigiconfig.h
        HINTS $ENV{STRIGIDIR}/include
    )

    find_library(STRIGI_STREAMS_LIBRARY
        NAMES streams
        HINTS $ENV{STRIGIDIR}/lib
    )

        find_library(STRIGI_STREAMANALYZER_LIBRARY
        NAMES streamanalyzer
        HINTS $ENV{STRIGIDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Strigi
    VERSION_VAR STRIGI_VERSION
    REQUIRED_VARS STRIGI_STREAMS_LIBRARY STRIGI_STREAMANALYZER_LIBRARY STRIGI_INCLUDE_DIR
)

mark_as_advanced(STRIGI_INCLUDE_DIR STRIGI_STREAMS_LIBRARY STRIGI_STREAMANALYZER_LIBRARY)
