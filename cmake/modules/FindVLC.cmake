# Try to find VLC, once done this will define:
#
#  VLC_FOUND - system has VLC
#  VLC_INCLUDES - the VLC include directory
#  VLC_LIBRARIES - the libraries needed to use VLC
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_VLC QUIET libvlc)

    set(VLC_INCLUDES ${PC_VLC_INCLUDE_DIRS})
    set(VLC_LIBRARIES ${PC_VLC_LIBRARIES})
endif()

set(VLC_VERSION ${PC_VLC_VERSION})

if(NOT VLC_INCLUDES OR NOT VLC_LIBRARIES)
    find_path(VLC_INCLUDES
        NAMES vlc/libvlc.h
        HINTS $ENV{VLCDIR}/include
    )

    find_library(VLC_LIBRARIES
        NAMES vlc
        HINTS $ENV{VLCDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VLC
    VERSION_VAR VLC_VERSION
    REQUIRED_VARS VLC_LIBRARIES VLC_INCLUDES
)

mark_as_advanced(VLC_INCLUDES VLC_LIBRARIES)
