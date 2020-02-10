# - Try to find libusb
#
# Once done this will define
#
#  LIBUSB_FOUND - system has libusb
#  LIBUSB_INCLUDES - the libusb include directory
#  LIBUSB_LIBRARIES - The libraries needed to use libusb
#
# Copyright (c) 2016-2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if(LIBUSB_INCLUDES AND LIBUSB_LIBRARIES)
    set(LIBUSB_FIND_QUIETLY TRUE)
endif()

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBUSB QUIET libusb-1.0)

    set(LIBUSB_INCLUDES ${PC_LIBUSB_INCLUDE_DIRS})
    set(LIBUSB_LIBRARIES ${PC_LIBUSB_LIBRARIES})
endif()

if(NOT LIBUSB_INCLUDES OR NOT LIBUSB_LIBRARIES)
    find_path(LIBUSB_INCLUDES
        NAMES libusb.h
        PATH_SUFFIXES libusb-1.0
        HINTS $ENV{LIBUSBDIR}/include
    )

    find_library(LIBUSB_LIBRARIES
        NAMES usb-1.0
        HINTS $ENV{LIBUSBDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibUSB
    VERSION_VAR PC_LIBUSB_VERSION
    REQUIRED_VARS LIBUSB_LIBRARIES LIBUSB_INCLUDES
)

mark_as_advanced(LIBUSB_INCLUDES LIBUSB_LIBRARIES)
