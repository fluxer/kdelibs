# - Try to find libusb
# Once done this will define
#
#  LIBUSB_FOUND - system has libusb
#  LIBUSB_INCLUDES - the libusb include directory
#  LIBUSB_LIBRARIES - The libraries needed to use libusb
#
# Copyright (c) 2016, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if(LIBUSB_INCLUDES AND LIBUSB_LIBRARIES)
    set(LIBUSB_FIND_QUIETLY TRUE)
endif()

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBUSB QUIET libusb-1.0)
endif()

find_path(LIBUSB_INCLUDES
    NAMES
    libusb.h
    PATH_SUFFIXES
    libusb-1.0
    HINTS
    $ENV{LIBUSBDIR}/include
    ${PC_LIBUSB_INCLUDEDIR}
    ${INCLUDE_INSTALL_DIR}
)

find_library(LIBUSB_LIBRARIES
    NAMES
    usb-1.0
    HINTS
    $ENV{FONTCONFIGDIR}/lib
    ${PC_LIBUSB_LIBDIR}
    ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibUSB DEFAULT_MSG LIBUSB_INCLUDES LIBUSB_LIBRARIES)

mark_as_advanced(LIBUSB_INCLUDES LIBUSB_LIBRARIES)
