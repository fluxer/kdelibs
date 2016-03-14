# - Try to find MPV
# Once done this will define
#
#  MPV_FOUND - system has MPV
#  MPV_INCLUDES - the MPV include directory
#  MPV_LIBRARIES - The libraries needed to use MPV
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if(MPV_INCLUDES AND MPV_LIBRARIES)
    set(MPV_FIND_QUIETLY TRUE)
endif()

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_MPV QUIET mpv)
endif()

find_path(MPV_INCLUDES
    NAMES
    client.h
    PATH_SUFFIXES mpv
    HINTS
    $ENV{MPVDIR}/include
    ${PC_MPV_INCLUDEDIR}
    /usr/include
    /usr/local/include
    ${INCLUDE_INSTALL_DIR}
)

find_library(MPV_LIBRARIES
    mpv
    HINTS
    $ENV{MPVDIR}/lib
    ${PC_MPV_LIBDIR}
    /usr/lib
    /usr/local/lib
    ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MPV DEFAULT_MSG MPV_INCLUDES MPV_LIBRARIES)

mark_as_advanced(MPV_INCLUDES MPV_LIBRARIES)
