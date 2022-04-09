# Try to find XCB on a Unix system, once done this will define:
#
#   XCB_FOUND       - True if xcb is available
#   XCB_LIBRARIES   - Link these to use xcb
#   XCB_INCLUDE_DIR - Include directory for xcb
#   XCB_DEFINITIONS - Compiler flags for using xcb
#
# In addition the following more fine grained variables will be defined:
#
#   XCB_XCB_FOUND        XCB_XCB_INCLUDE_DIR        XCB_XCB_LIBRARIES
#   XCB_COMPOSITE_FOUND  XCB_COMPOSITE_INCLUDE_DIR  XCB_COMPOSITE_LIBRARIES
#   XCB_DAMAGE_FOUND     XCB_DAMAGE_INCLUDE_DIR     XCB_DAMAGE_LIBRARIES
#   XCB_XFIXES_FOUND     XCB_XFIXES_INCLUDE_DIR     XCB_XFIXES_LIBRARIES
#   XCB_RENDER_FOUND     XCB_RENDER_INCLUDE_DIR     XCB_RENDER_LIBRARIES
#   XCB_RANDR_FOUND      XCB_RANDR_INCLUDE_DIR      XCB_RANDR_LIBRARIES
#   XCB_SHAPE_FOUND      XCB_SHAPE_INCLUDE_DIR      XCB_SHAPE_LIBRARIES
#   XCB_SYNC_FOUND       XCB_SYNC_INCLUDE_DIR       XCB_SYNC_LIBRARIES
#   XCB_RENDERUTIL_FOUND XCB_RENDERUTIL_INCLUDE_DIR XCB_RENDERUTIL_LIBRARIES
#   XCB_KEYSYMS_FOUND    XCB_KEYSYMS_INCLUDE_DIR    XCB_KEYSYMS_LIBRARIES
#
# Copyright (c) 2012 Fredrik Höglund <fredrik@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (NOT WIN32)
    IF (XCB_INCLUDE_DIR AND XCB_LIBRARIES)
        # In the cache already
        SET(XCB_FIND_QUIETLY TRUE)
    ENDIF ()

    # Use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    FIND_PACKAGE(PkgConfig)
    PKG_CHECK_MODULES(PKG_XCB QUIET
        xcb xcb-util xcb-composite xcb-xfixes xcb-damage xcb-render xcb-randr
        xcb-shape xcb-sync xcb-renderutil xcb-keysyms
    )

    SET(XCB_DEFINITIONS ${PKG_XCB_CFLAGS})

    FIND_PATH(XCB_XCB_INCLUDE_DIR         NAMES xcb/xcb.h             HINTS ${PKG_XCB_INCLUDE_DIRS})
    FIND_PATH(XCB_COMPOSITE_INCLUDE_DIR   NAMES xcb/composite.h       HINTS ${PKG_XCB_INCLUDE_DIRS})
    FIND_PATH(XCB_XFIXES_INCLUDE_DIR      NAMES xcb/xfixes.h          HINTS ${PKG_XCB_INCLUDE_DIRS})
    FIND_PATH(XCB_DAMAGE_INCLUDE_DIR      NAMES xcb/damage.h          HINTS ${PKG_XCB_INCLUDE_DIRS})
    FIND_PATH(XCB_RENDER_INCLUDE_DIR      NAMES xcb/render.h          HINTS ${PKG_XCB_INCLUDE_DIRS})
    FIND_PATH(XCB_RANDR_INCLUDE_DIR       NAMES xcb/randr.h           HINTS ${PKG_XCB_INCLUDE_DIRS})
    FIND_PATH(XCB_SHAPE_INCLUDE_DIR       NAMES xcb/shape.h           HINTS ${PKG_XCB_INCLUDE_DIRS})
    FIND_PATH(XCB_SYNC_INCLUDE_DIR        NAMES xcb/sync.h            HINTS ${PKG_XCB_INCLUDE_DIRS})
    FIND_PATH(XCB_RENDERUTIL_INCLUDE_DIR  NAMES xcb/xcb_renderutil.h  HINTS ${PKG_XCB_INCLUDE_DIRS})
    FIND_PATH(XCB_KEYSYMS_INCLUDE_DIR     NAMES xcb/xcb_keysyms.h     HINTS ${PKG_XCB_INCLUDE_DIRS})

    FIND_LIBRARY(XCB_XCB_LIBRARIES        NAMES xcb              HINTS ${PKG_XCB_LIBRARY_DIRS})
    FIND_LIBRARY(XCB_COMPOSITE_LIBRARIES  NAMES xcb-composite    HINTS ${PKG_XCB_LIBRARY_DIRS})
    FIND_LIBRARY(XCB_DAMAGE_LIBRARIES     NAMES xcb-damage       HINTS ${PKG_XCB_LIBRARY_DIRS})
    FIND_LIBRARY(XCB_XFIXES_LIBRARIES     NAMES xcb-xfixes       HINTS ${PKG_XCB_LIBRARY_DIRS})
    FIND_LIBRARY(XCB_RENDER_LIBRARIES     NAMES xcb-render       HINTS ${PKG_XCB_LIBRARY_DIRS})
    FIND_LIBRARY(XCB_RANDR_LIBRARIES      NAMES xcb-randr        HINTS ${PKG_XCB_LIBRARY_DIRS})
    FIND_LIBRARY(XCB_SHAPE_LIBRARIES      NAMES xcb-shape        HINTS ${PKG_XCB_LIBRARY_DIRS})
    FIND_LIBRARY(XCB_SYNC_LIBRARIES       NAMES xcb-sync         HINTS ${PKG_XCB_LIBRARY_DIRS})
    FIND_LIBRARY(XCB_RENDERUTIL_LIBRARIES NAMES xcb-render-util  HINTS ${PKG_XCB_LIBRARY_DIRS})
    FIND_LIBRARY(XCB_KEYSYMS_LIBRARIES    NAMES xcb-keysyms      HINTS ${PKG_XCB_LIBRARY_DIRS})

    set(XCB_INCLUDE_DIR
        ${XCB_XCB_INCLUDE_DIR} ${XCB_COMPOSITE_INCLUDE_DIR} ${XCB_XFIXES_INCLUDE_DIR}
        ${XCB_DAMAGE_INCLUDE_DIR} ${XCB_RENDER_INCLUDE_DIR} ${XCB_RANDR_INCLUDE_DIR}
        ${XCB_SHAPE_INCLUDE_DIR} ${XCB_SYNC_INCLUDE_DIR} ${XCB_RENDERUTIL_INCLUDE_DIR}
        ${XCB_KEYSYMS_INCLUDE_DIR}
    )

    set(XCB_LIBRARIES
        ${XCB_XCB_LIBRARIES} ${XCB_COMPOSITE_LIBRARIES} ${XCB_XFIXES_LIBRARIES}
        ${XCB_DAMAGE_LIBRARIES} ${XCB_RENDER_LIBRARIES} ${XCB_RANDR_LIBRARIES}
        ${XCB_SHAPE_LIBRARIES} ${XCB_SYNC_LIBRARIES} ${XCB_RENDERUTIL_LIBRARIES}
        ${XCB_KEYSYMS_LIBRARIES}
    )

    list(REMOVE_DUPLICATES XCB_INCLUDE_DIR)

    include(FindPackageHandleStandardArgs)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB_XCB         DEFAULT_MSG  XCB_XCB_LIBRARIES         XCB_XCB_INCLUDE_DIR)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB_COMPOSITE   DEFAULT_MSG  XCB_COMPOSITE_LIBRARIES   XCB_COMPOSITE_INCLUDE_DIR)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB_DAMAGE      DEFAULT_MSG  XCB_DAMAGE_LIBRARIES      XCB_DAMAGE_INCLUDE_DIR)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB_XFIXES      DEFAULT_MSG  XCB_XFIXES_LIBRARIES      XCB_XFIXES_INCLUDE_DIR)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB_RENDER      DEFAULT_MSG  XCB_RENDER_LIBRARIES      XCB_RENDER_INCLUDE_DIR)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB_RANDR       DEFAULT_MSG  XCB_RANDR_LIBRARIES       XCB_RANDR_INCLUDE_DIR)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB_SHAPE       DEFAULT_MSG  XCB_SHAPE_LIBRARIES       XCB_SHAPE_INCLUDE_DIR)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB_SYNC        DEFAULT_MSG  XCB_SYNC_LIBRARIES        XCB_SYNC_INCLUDE_DIR)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB_RENDERUTIL  DEFAULT_MSG  XCB_RENDERUTIL_LIBRARIES  XCB_RENDERUTIL_INCLUDE_DIR)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB_KEYSYMS     DEFAULT_MSG  XCB_KEYSYMS_LIBRARIES     XCB_KEYSYMS_INCLUDE_DIR)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB             DEFAULT_MSG  XCB_LIBRARIES             XCB_INCLUDE_DIR)

    MARK_AS_ADVANCED(
        XCB_INCLUDE_DIR             XCB_LIBRARIES
        XCB_XCB_INCLUDE_DIR         XCB_XCB_LIBRARIES
        XCB_COMPOSITE_INCLUDE_DIR   XCB_COMPOSITE_LIBRARIES
        XCB_DAMAGE_INCLUDE_DIR      XCB_DAMAGE_LIBRARIES
        XCB_XFIXES_INCLUDE_DIR      XCB_XFIXES_LIBRARIES
        XCB_RENDER_INCLUDE_DIR      XCB_RENDER_LIBRARIES
        XCB_RANDR_INCLUDE_DIR       XCB_RANDR_LIBRARIES
        XCB_SHAPE_INCLUDE_DIR       XCB_SHAPE_LIBRARIES
        XCB_SYNC_INCLUDE_DIR        XCB_SYNC_LIBRARIES
        XCB_RENDERUTIL_INCLUDE_DIR  XCB_RENDERUTIL_LIBRARIES
        XCB_KEYSYMS_INCLUDE_DIR     XCB_KEYSYMS_LIBRARIES
    )
ENDIF (NOT WIN32)
