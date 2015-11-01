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

project(libkexiv2)

add_definitions(
    ${QT_QTDBUS_DEFINITIONS}
    -DKDE_DEFAULT_DEBUG_AREA=51003
)

include_directories(
    ${QDBUS_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
)

find_package(Exiv2 REQUIRED VERSION "0.21")
set_package_properties(Exiv2 PROPERTIES
    DESCRIPTION "Image metadata library and tools"
    URL "http://www.exiv2.org"
    PURPOSE "Required to build libkexiv2"
)

# =======================================================
# Set env. variables accordinly.

set(KEXIV2_LIB_VERSION_STRING "${KDE_VERSION_MAJOR}.${KDE_VERSION_MINOR}.${KDE_VERSION_RELEASE}")
set(KEXIV2_LIB_VERSION_ID "0x0${KDE_VERSION_MAJOR}0${KDE_VERSION_MINOR}0${KDE_VERSION_RELEASE}")
set(KEXIV2_LIB_SO_VERSION_STRING "${KDE_VERSION_MAJOR}.${KDE_VERSION_MINOR}.${KDE_VERSION_RELEASE}")

# =======================================================

add_subdirectory(libkexiv2)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/libkexiv2.pc.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libkexiv2.pc
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/libkexiv2.pc
    DESTINATION ${LIB_INSTALL_DIR}/pkgconfig
)

