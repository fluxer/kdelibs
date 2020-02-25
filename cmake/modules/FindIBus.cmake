# - Try to find IBUS
#
# Once done this will define
#
#  IBUS_FOUND - system has IBUS
#  IBUS_INCLUDE_DIR - the IBUS include directory
#  IBUS_LIBRARIES -  the libraries needed to use IBUS
#  IBUS_DEFINITIONS - compiler switches required for using IBUS
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_IBUS QUIET ibus-1.0)

    set(IBUS_INCLUDE_DIR ${PC_IBUS_INCLUDE_DIRS})
    set(IBUS_LIBRARIES ${PC_IBUS_LIBRARIES})
endif()

set(IBUS_VERSION ${PC_IBUS_VERSION})
set(IBUS_DEFINITIONS ${PC_IBUS_CFLAGS_OTHER})

if(NOT IBUS_INCLUDE_DIR OR NOT IBUS_LIBRARIES)
    find_path(IBUS_INCLUDE_DIR
        NAMES ibus.h
        PATH_SUFFIXES ibus-1.0
        HINTS $ENV{IBUSDIR}/include
    )

    find_library(IBUS_LIBRARIES
        NAMES ibus-1.0 ibus
        HINTS $ENV{IBUSDIR}/lib
    )

    # IBUS requires Glib2 (gio-2.0, gobject-2.0 and glib-2.0)
    if(IBUS_INCLUDE_DIR AND IBUS_LIBRARIES)
        find_package(GIO REQUIRED)
        find_package(GLIB2 REQUIRED)
        set(IBUS_INCLUDE_DIR ${IBUS_INCLUDE_DIR} ${GIO_INCLUDE_DIR} ${GLIB2_INCLUDE_DIR})
        set(IBUS_LIBRARIES ${IBUS_LIBRARIES} ${GIO_LIBRARIES} ${GLIB2_LIBRARIES})
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IBUS
    VERSION_VAR IBUS_VERSION
    REQUIRED_VARS IBUS_LIBRARIES IBUS_INCLUDE_DIR
)

mark_as_advanced(IBUS_INCLUDE_DIR IBUS_LIBRARIES)
