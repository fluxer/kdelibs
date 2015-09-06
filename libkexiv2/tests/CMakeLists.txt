# ===========================================================
#
# This file is a part of digiKam project
# <a href="http://www.digikam.org">http://www.digikam.org</a>
#
# @date   2006-09-15
# @brief  Exiv2 library interface for KDE
#
# @author Copyright (C) 2006-2012 by Gilles Caulier
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

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../libkexiv2)

SET(setiptcpreview_SRCS setiptcpreview.cpp)
KDE4_ADD_MANUAL_TEST(kexiv2-setiptcpreview ${setiptcpreview_SRCS})
TARGET_LINK_LIBRARIES(kexiv2-setiptcpreview kexiv2 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

SET(loadfromba_SRCS loadfromba.cpp)
KDE4_ADD_MANUAL_TEST(kexiv2-loadfromba ${loadfromba_SRCS})
TARGET_LINK_LIBRARIES(kexiv2-loadfromba kexiv2 ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

SET(erasetag_SRCS erasetag.cpp)
KDE4_ADD_MANUAL_TEST(kexiv2-erasetag ${erasetag_SRCS})
TARGET_LINK_LIBRARIES(kexiv2-erasetag kexiv2 ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

SET(printtagslist_SRCS printtagslist.cpp)
KDE4_ADD_MANUAL_TEST(kexiv2-printtagslist ${printtagslist_SRCS})
TARGET_LINK_LIBRARIES(kexiv2-printtagslist kexiv2 ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

SET(usexmpsidecar_SRCS usexmpsidecar.cpp)
KDE4_ADD_MANUAL_TEST(kexiv2-usexmpsidecar ${usexmpsidecar_SRCS})
TARGET_LINK_LIBRARIES(kexiv2-usexmpsidecar kexiv2 ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

SET(readimagewritexmpsidecar_SRCS readimagewritexmpsidecar.cpp)
KDE4_ADD_MANUAL_TEST(kexiv2-readimagewritexmpsidecar ${readimagewritexmpsidecar_SRCS})
TARGET_LINK_LIBRARIES(kexiv2-readimagewritexmpsidecar kexiv2 ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

SET(setxmpface_SRCS setxmpface.cpp)
KDE4_ADD_MANUAL_TEST(kexiv2-setxmpface ${setxmpface_SRCS})
TARGET_LINK_LIBRARIES(kexiv2-setxmpface kexiv2 ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
