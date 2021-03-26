# Try to find UDev, once done this will define:
#
#  UDEV_FOUND - system has UDev
#  UDEV_INCLUDES - the UDev include directory
#  UDEV_LIBRARIES - the libraries needed to use UDev
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_UDEV QUIET libudev)

    set(UDEV_INCLUDES ${PC_UDEV_INCLUDE_DIRS})
    set(UDEV_LIBRARIES ${PC_UDEV_LIBRARIES})
endif()

set(UDEV_VERSION ${PC_UDEV_VERSION})

if(NOT UDEV_INCLUDES OR NOT UDEV_LIBRARIES)
    find_path(UDEV_INCLUDES
        NAMES libudev.h
        HINTS $ENV{UDEVDIR}/include
    )

    find_library(UDEV_LIBRARIES
        NAMES udev
        HINTS $ENV{UDEVDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UDev
    VERSION_VAR UDEV_VERSION
    REQUIRED_VARS UDEV_LIBRARIES UDEV_INCLUDES
)

mark_as_advanced(UDEV_INCLUDES UDEV_LIBRARIES)