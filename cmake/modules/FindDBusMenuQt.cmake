# - Try to find DBusMenuQt library
#
# Once done this will define
#
#  DBUSMENUQT_FOUND - system has DBusMenuQt
#  DBUSMENUQT_INCLUDE_DIR - the DBusMenuQt include directory
#  DBUSMENUQT_LIBRARIES - the libraries needed to use DBusMenuQt
#  DBUSMENUQT_DEFINITIONS - compiler switches required for using DBusMenuQt
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(DBUSMENU_QT_NAMES dbusmenu-katie dbusmenu-qt dbusmenu-qtd)

if(NOT WIN32)
    find_package(PkgConfig)
    foreach(name ${DBUSMENU_QT_NAMES})
        if(NOT PC_DBUSMENUQT_FOUND)
            pkg_check_modules(PC_DBUSMENUQT QUIET ${name})

            set(DBUSMENUQT_INCLUDE_DIR ${PC_DBUSMENUQT_INCLUDE_DIRS})
            set(DBUSMENUQT_LIBRARIES ${PC_DBUSMENUQT_LIBRARIES})
        endif()
    endforeach()
endif()

set(DBUSMENUQT_VERSION ${PC_DBUSMENUQT_VERSION})
set(DBUSMENUQT_DEFINITIONS ${PC_DBUSMENUQT_CFLAGS_OTHER})

if(NOT DBUSMENUQT_INCLUDE_DIR OR NOT DBUSMENUQT_LIBRARIES)
    find_path(DBUSMENUQT_INCLUDE_DIR
        NAMES dbusmenuexporter.h
        PATH_SUFFIXES ${DBUSMENU_QT_NAMES}
        HINTS $ENV{DBUSMENUQTDIR}/include
    )

    find_library(DBUSMENUQT_LIBRARIES
        NAMES ${DBUSMENU_QT_NAMES}
        HINTS $ENV{DBUSMENUQTDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DBusMenuQt
    VERSION_VAR DBUSMENUQT_VERSION
    REQUIRED_VARS DBUSMENUQT_LIBRARIES DBUSMENUQT_INCLUDE_DIR
)

mark_as_advanced(DBUSMENUQT_INCLUDE_DIR DBUSMENUQT_LIBRARIES)
