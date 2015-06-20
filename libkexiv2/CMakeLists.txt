# ===========================================================
#
# This file is a part of digiKam project
# <a href="http://www.digikam.org">http://www.digikam.org</a>
#
# @date   2006-09-15
# @brief  Exiv2 library interface for KDE
#
# @author Copyright (C) 2006-2014 by Gilles Caulier
#         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
#
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation;
# either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# ============================================================

PROJECT(libkexiv2)

MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS "Starting CMake configuration for: libkexiv2")

FIND_PACKAGE(KDE4Internal REQUIRED 4.14.3)

INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)
INCLUDE(MacroOptionalAddSubdirectory)
INCLUDE(MacroOptionalFindPackage)
INCLUDE(FindPackageHandleStandardArgs)

ADD_DEFINITIONS (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
INCLUDE_DIRECTORIES (${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

SET(LIBKEXIV2_AREA_CODE_GENERAL 51003)
ADD_DEFINITIONS(-DKDE_DEFAULT_DEBUG_AREA=${LIBKEXIV2_AREA_CODE_GENERAL})

SET(EXIV2_MIN_VERSION "0.21")
FIND_PACKAGE(Exiv2)
MACRO_LOG_FEATURE(EXIV2_FOUND "Exiv2" "Required to build libkexiv2." "http://www.exiv2.org"
                  TRUE ${EXIV2_MIN_VERSION} "")

# =======================================================
# Set env. variables accordinly.

SET(KEXIV2_LIB_VERSION_STRING "${KDE_VERSION_MAJOR}.${KDE_VERSION_MINOR}.${KDE_VERSION_RELEASE}")
SET(KEXIV2_LIB_VERSION_ID "0x0${KDE_VERSION_MAJOR}0${KDE_VERSION_MINOR}0${KDE_VERSION_RELEASE}")
SET(KEXIV2_LIB_SO_VERSION_STRING "${KDE_VERSION_MAJOR}.${KDE_VERSION_MINOR}.${KDE_VERSION_RELEASE}")

# =======================================================

IF (EXIV2_FOUND)
  ADD_SUBDIRECTORY(libkexiv2)

  IF(KDE4_BUILD_TESTS)
    MACRO_OPTIONAL_ADD_SUBDIRECTORY(tests)
  ENDIF(KDE4_BUILD_TESTS)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libkexiv2.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libkexiv2.pc)
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libkexiv2.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
ENDIF (EXIV2_FOUND)

MACRO_DISPLAY_FEATURE_LOG()
