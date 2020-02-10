# - Try to find libwebp
#
# Once done this will define
#
#  WEBP_FOUND - system has libwebp
#  WEBP_INCLUDES - the libwebp include directory
#  WEBP_LIBRARIES - The libraries needed to use libwebp
#
# Copyright (c) 2015-2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(WEBP_INCLUDES AND WEBP_LIBRARIES)
    set(WEBP_FIND_QUIETLY TRUE)
endif()

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_WEBP QUIET libwebp)

    set(WEBP_INCLUDES ${PC_WEBP_INCLUDE_DIRS})
    set(WEBP_LIBRARIES ${PC_WEBP_LIBRARIES})
endif()

if(NOT WEBP_INCLUDES OR NOT WEBP_LIBRARIES)
    find_path(WEBP_INCLUDES
        NAMES encode.h decode.h
        PATH_SUFFIXES webp
        HINTS $ENV{WEBPDIR}/include
    )

    find_library(WEBP_LIBRARIES
        NAMES webp
        HINTS $ENV{WEBPDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WebP
    VERSION_VAR PC_WEBP_VERSION
    REQUIRED_VARS WEBP_LIBRARIES WEBP_INCLUDES
)

mark_as_advanced(WEBP_INCLUDES WEBP_LIBRARIES)
