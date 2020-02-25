# - Try to find Exiv2 library
#
# Once done this will define
#
#  EXIV2_FOUND - system has Exiv2
#  EXIV2_INCLUDE_DIR - the Exiv2 include directory
#  EXIV2_LIBRARIES - the libraries needed to use Exiv2
#  EXIV2_DEFINITIONS - compiler switches required for using Exiv2
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_EXIV2 QUIET exiv2)

    set(EXIV2_INCLUDE_DIR ${PC_EXIV2_INCLUDE_DIRS})
    set(EXIV2_LIBRARIES ${PC_EXIV2_LIBRARIES})
endif()

set(EXIV2_VERSION ${PC_EXIV2_VERSION})
set(EXIV2_DEFINITIONS ${PC_EXIV2_CFLAGS_OTHER})

if(NOT EXIV2_INCLUDE_DIR OR NOT EXIV2_LIBRARIES)
    find_path(EXIV2_INCLUDE_DIR
        NAMES exiv2/exif.hpp
        HINTS $ENV{EXIV2DIR}/include
    )

    find_library(EXIV2_LIBRARIES
        NAMES exiv2
        HINTS $ENV{EXIV2DIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Exiv2
    VERSION_VAR EXIV2_VERSION
    REQUIRED_VARS EXIV2_LIBRARIES EXIV2_INCLUDE_DIR
)

mark_as_advanced(EXIV2_INCLUDE_DIR EXIV2_LIBRARIES)
