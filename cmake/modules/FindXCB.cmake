# - Try to find XCB
# Once done this will define
#
#  XCB_FOUND - system has XCB
#  XCB_INCLUDES - the libxcb include directory
#  XCB_LIBRARIES - The libraries needed to use libxcb
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if(XCB_INCLUDES AND XCB_LIBRARIES)
    set(XCB_FIND_QUIETLY TRUE)
endif(XCB_INCLUDES AND XCB_LIBRARIES)

find_path(XCB_INCLUDES
    NAMES
    xcb.h
    PATH_SUFFIXES xcb
    HINTS
    $ENV{NASDIR}/include
    /usr/include
    /usr/local/include
    ${INCLUDE_INSTALL_DIR}
)

find_library(XCB_LIBRARIES
    xcb
    HINTS
    $ENV{NASDIR}/lib
    /usr/lib
    /usr/local/lib
    ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XCB DEFAULT_MSG XCB_INCLUDES XCB_LIBRARIES)

mark_as_advanced(XCB_INCLUDES XCB_LIBRARIES)
