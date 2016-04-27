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

include_directories(${EXIV2_INCLUDE_DIR})

add_definitions(${EXIV2_DEFINITIONS} ${KDE4_ENABLE_EXCEPTIONS})

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

# This was used to enable catching of exceptions thrown by libexiv2.
# We use now #pragma GCC visibility push(default) around the headers
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default")

set(kexiv2_LIB_SRCS
    kexiv2.cpp
    kexiv2_p.cpp
    kexiv2image.cpp
    kexiv2comments.cpp
    kexiv2exif.cpp
    kexiv2iptc.cpp
    kexiv2gps.cpp
    kexiv2xmp.cpp
    kexiv2previews.cpp
    altlangstredit.cpp
    msgtextedit.cpp
    countryselector.cpp
    subjectwidget.cpp
    rotationmatrix.cpp
)

add_library(kexiv2 SHARED ${kexiv2_LIB_SRCS})

target_link_libraries(kexiv2
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${EXIV2_LIBRARIES}
)

set_target_properties(kexiv2 PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS kexiv2
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(
    FILES
    topicset.iptc-subjectcode.xml
    DESTINATION ${DATA_INSTALL_DIR}/libkexiv2/data
)

generate_export_header(kexiv2)

install(
    FILES
    kexiv2.h
    kexiv2data.h
    kexiv2previews.h
    msgtextedit.h
    subjectwidget.h
    altlangstredit.h
    countryselector.h
    rotationmatrix.h
    ${CMAKE_CURRENT_BINARY_DIR}/kexiv2_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/libkexiv2
    COMPONENT Devel
)
