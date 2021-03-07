# Try to find libwebp, once done this will define:
#
#  WEBP_FOUND - system has libwebp
#  WEBP_INCLUDES - the libwebp include directory
#  WEBP_LIBRARIES - the libraries needed to use libwebp
#
# Copyright (c) 2015 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_WEBP QUIET libwebp)

    set(WEBP_INCLUDES ${PC_WEBP_INCLUDE_DIRS})
    set(WEBP_LIBRARIES ${PC_WEBP_LIBRARIES})
endif()

set(WEBP_VERSION ${PC_WEBP_VERSION})

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
    VERSION_VAR WEBP_VERSION
    REQUIRED_VARS WEBP_LIBRARIES WEBP_INCLUDES
)

mark_as_advanced(WEBP_INCLUDES WEBP_LIBRARIES)
