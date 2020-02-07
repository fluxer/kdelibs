# - Try to find DBusMenuQt library
# Once done this will define
#
#  DBUSMENUQT_FOUND - system has DBusMenuQt
#  DBUSMENUQT_INCLUDE_DIR - the DBusMenuQt include directory
#  DBUSMENUQT_LIBRARIES - the libraries needed to use DBusMenuQt
#  DBUSMENUQT_DEFINITIONS - Compiler switches required for using DBusMenuQt
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(DBUSMENU_QT_NAMES dbusmenu-katie dbusmenu-qt dbusmenu-qtd)

find_package(PkgConfig)
foreach(name ${DBUSMENU_QT_NAMES})
    if(NOT PC_DBUSMENUQT_FOUND)
        pkg_check_modules(PC_DBUSMENUQT QUIET ${name})
    endif()
endforeach()

set(DBUSMENUQT_DEFINITIONS ${PC_DBUSMENUQT_CFLAGS_OTHER})

find_path(DBUSMENUQT_INCLUDE_DIR
    NAMES dbusmenuexporter.h
    HINTS ${PC_DBUSMENUQT_INCLUDEDIR}
    PATH_SUFFIXES ${DBUSMENU_QT_NAMES}
)

find_library(DBUSMENUQT_LIBRARIES
    NAMES ${DBUSMENU_QT_NAMES}
    HINTS ${PC_DBUSMENUQT_LIBDIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DBusMenuQt
    REQUIRED_VARS DBUSMENUQT_LIBRARIES DBUSMENUQT_INCLUDE_DIR
    VERSION_VAR PC_DBUSMENUQT_VERSION
)

mark_as_advanced(DBUSMENUQT_INCLUDE_DIR DBUSMENUQT_LIBRARIES)
