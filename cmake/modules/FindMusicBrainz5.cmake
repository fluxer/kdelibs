# - Try to find MusicBrainz
# Once done this will define
#
#  MUSICBRAINZ5_FOUND - system has MusicBrainz
#  MUSICBRAINZ5_INCLUDE_DIR - the MusicBrainz include directory
#  MUSICBRAINZ5_LIBRARIES - The libraries needed to use MusicBrainz
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if(MUSICBRAINZ5_INCLUDE_DIR AND MUSICBRAINZ5_LIBRARIES)
    set(MUSICBRAINZ5_FIND_QUIETLY TRUE)
endif()

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_MUSICBRAINZ5 QUIET libmusicbrainz5)
endif()

find_path(MUSICBRAINZ5_INCLUDE_DIR
    NAMES
    musicbrainz5/Disc.h
    HINTS
    $ENV{MUSICBRAINZ5DIR}/include
    ${PC_MUSICBRAINZ5_INCLUDEDIR}
    ${INCLUDE_INSTALL_DIR}
)

find_library(MUSICBRAINZ5_LIBRARIES
    musicbrainz5cc musicbrainz5
    HINTS
    $ENV{MUSICBRAINZ5DIR}/lib
    ${PC_MUSICBRAINZ5_LIBDIR}
    ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MusicBrainz5
    VERSION_VAR PC_MUSICBRAINZ5_VERSION
    REQUIRED_VARS MUSICBRAINZ5_LIBRARIES MUSICBRAINZ5_INCLUDE_DIR
)

mark_as_advanced(MUSICBRAINZ5_INCLUDE_DIR MUSICBRAINZ5_LIBRARIES)
