# Try to find Gphoto2 library, once done this will define:
#
#  GPHOTO2_FOUND - system has Gphoto2
#  GPHOTO2_INCLUDE_DIR - the Gphoto2 include directory
#  GPHOTO2_LIBRARIES - the libraries needed to use Gphoto2
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_GPHOTO2 QUIET libgphoto2)

set(GPHOTO2_INCLUDE_DIR ${PC_GPHOTO2_INCLUDE_DIRS})
set(GPHOTO2_LIBRARIES ${PC_GPHOTO2_LIBRARIES})
set(GPHOTO2_VERSION ${PC_GPHOTO2_VERSION})

if(NOT GPHOTO2_INCLUDE_DIR OR NOT GPHOTO2_LIBRARIES)
    find_path(GPHOTO2_INCLUDE_DIR
        NAMES gphoto2.h
        PATH_SUFFIXES gphoto2
        HINTS $ENV{GPHOTO2DIR}/include
    )

    find_library(GPHOTO2_LIBRARIES
        NAMES gphoto2
        HINTS $ENV{GPHOTO2DIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gphoto2
    VERSION_VAR GPHOTO2_VERSION
    REQUIRED_VARS GPHOTO2_LIBRARIES GPHOTO2_INCLUDE_DIR
)

mark_as_advanced(GPHOTO2_INCLUDE_DIR GPHOTO2_LIBRARIES)
