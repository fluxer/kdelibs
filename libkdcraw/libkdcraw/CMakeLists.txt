# ===========================================================
#
# This file is was part of digiKam project
# <a href="http://www.digikam.org">http://www.digikam.org</a>
#
# @date   2006-12-09
# @brief  a tread-safe libraw C++ program interface for KDE
#
# @author Copyright (C) 2006-2013 by Gilles Caulier
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

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

set(kdcraw_LIB_SRCS
    kdcraw.cpp
    kdcraw_p.cpp
    dcrawsettingswidget.cpp
    dcrawinfocontainer.cpp
    rawdecodingsettings.cpp
    rcombobox.cpp
    rnuminput.cpp
    rexpanderbox.cpp
    ractionthreadbase.cpp
    ractionthreadbase_p.cpp
    squeezedcombobox.cpp
)

add_library(kdcraw ${LIBRARY_TYPE} ${kdcraw_LIB_SRCS})

target_link_libraries(kdcraw PRIVATE
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${KDE4_THREADWEAVER_LIBS}
)

target_link_libraries(kdcraw PUBLIC
    ${LibRaw_r_LIBRARIES}
)

set_target_properties(kdcraw PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS kdcraw ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    rawdecodingsettings.h
    kdcraw.h
    dcrawsettingswidget.h
    rnuminput.h
    rcombobox.h
    rexpanderbox.h
    ractionthreadbase.h
    squeezedcombobox.h
    dcrawinfocontainer.h
    rawfiles.h
    libkdcraw_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/libkdcraw
    COMPONENT Devel
)
