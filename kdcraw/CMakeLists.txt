# ===========================================================
#
# This file is a part of digiKam project
# <a href="http://www.digikam.org">http://www.digikam.org</a>
#
# @date   2006-12-09
# @brief  a tread-safe libraw C++ program interface for KDE
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

PROJECT(libkdcraw)

IF(EXISTS "${LIBRAW_INCLUDE_DIR}/libraw_config.h")
    ADD_DEFINITIONS(-DLIBRAW_HAS_CONFIG)
    MESSAGE(STATUS "LibRaw config file exists: yes")
ELSE()
    MESSAGE(STATUS "LibRaw config file exists: no")
ENDIF()

ADD_DEFINITIONS(
    ${QT_QTDBUS_DEFINITIONS}
    ${LIBRAW_DEFINITIONS}
    -DKDE_DEFAULT_DEBUG_AREA=51002
)

INCLUDE_DIRECTORIES(
    # for the export header
    ${CMAKE_CURRENT_BINARY_DIR}/libkdcraw
    ${QDBUS_INCLUDE_DIRS} 
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KIO_INCLUDES}
    ${LIBRAW_INCLUDE_DIR}
)

# ==================================================================================================
# Set env. variables accordinly.

SET(DCRAW_LIB_VERSION_STRING "${KDE_VERSION_MAJOR}.${KDE_VERSION_MINOR}.${KDE_VERSION_RELEASE}")
SET(DCRAW_LIB_VERSION_ID "0x0${KDE_VERSION_MAJOR}0${KDE_VERSION_MINOR}0${KDE_VERSION_RELEASE}")
SET(DCRAW_LIB_SO_VERSION_STRING "${DCRAW_LIB_VERSION_STRING}")

# ==================================================================================================

ADD_SUBDIRECTORY(libkdcraw)
ADD_SUBDIRECTORY(icons)
ADD_SUBDIRECTORY(profiles)

IF(ENABLE_TESTING)
    ADD_SUBDIRECTORY(tests)
ENDIF()
