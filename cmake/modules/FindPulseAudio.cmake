# - Try to find PulseAudio
#
# Once done this will define
#
#  PULSEAUDIO_FOUND - system has PulseAudio
#  PULSEAUDIO_INCLUDE_DIR - the PulseAudio include directory
#  PULSEAUDIO_LIBRARY - the libraries needed to use PulseAudio
#  PULSEAUDIO_MAINLOOP_LIBRARY - the mainloop libraries needed to use PulseAudio
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_PULSEAUDIO QUIET libpulse)
    pkg_check_modules(PC_PULSEAUDIO_MAINLOOP QUIET libpulse-mainloop-glib)

    set(PULSEAUDIO_INCLUDE_DIR ${PC_PULSEAUDIO_INCLUDE_DIRS} ${PC_PULSEAUDIO_MAINLOOP_INCLUDE_DIRS})
    set(PULSEAUDIO_LIBRARY ${PC_PULSEAUDIO_LIBRARIES})
    set(PULSEAUDIO_MAINLOOP_LIBRARY ${PC_PULSEAUDIO_MAINLOOP_LIBRARIES})
endif()

set(PULSEAUDIO_VERSION ${PC_PULSEAUDIO_VERSION})

if(NOT PULSEAUDIO_INCLUDE_DIR OR NOT PULSEAUDIO_LIBRARY OR NOT PULSEAUDIO_MAINLOOP_LIBRARY)
    find_path(PULSEAUDIO_INCLUDE_DIR
        NAMES pulse/pulseaudio.h
        HINTS $ENV{PULSEAUDIODIR}/include
    )

    find_library(PULSEAUDIO_LIBRARY
        NAMES pulse
        HINTS $ENV{PULSEAUDIODIR}/lib
    )

    find_library(PULSEAUDIO_MAINLOOP_LIBRARY
        NAMES pulse-mainloop pulse-mainloop-glib
        HINTS $ENV{PULSEAUDIODIR}/lib
    )

    # PulseAudio requires Glib2
    if(PULSEAUDIO_INCLUDE_DIR AND PULSEAUDIO_LIBRARY AND PULSEAUDIO_MAINLOOP_LIBRARY)
        find_package(GLIB2 REQUIRED)
        set(PULSEAUDIO_INCLUDE_DIR ${PULSEAUDIO_INCLUDE_DIR} ${GLIB2_INCLUDE_DIR})
        set(PULSEAUDIO_MAINLOOP_LIBRARY ${PULSEAUDIO_LIBRARY} ${GLIB2_LIBRARIES})
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PulseAudio
    VERSION_VAR PULSEAUDIO_VERSION
    REQUIRED_VARS PULSEAUDIO_LIBRARY PULSEAUDIO_MAINLOOP_LIBRARY PULSEAUDIO_INCLUDE_DIR
)

mark_as_advanced(PULSEAUDIO_INCLUDE_DIR PULSEAUDIO_LIBRARY PULSEAUDIO_MAINLOOP_LIBRARY)
