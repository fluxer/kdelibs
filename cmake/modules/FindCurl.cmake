# Try to find curl library, once done this will define:
#
#  CURL_FOUND - system has curl
#  CURL_INCLUDE_DIR - the curl include directory
#  CURL_LIBRARIES - the libraries needed to use curl
#  CURL_DEFINITIONS - compiler switches required for using curl
#
# Copyright (c) 2022 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_CURL QUIET libcurl)

    set(CURL_INCLUDE_DIR ${PC_CURL_INCLUDE_DIRS})
    set(CURL_LIBRARIES ${PC_CURL_LIBRARIES})
endif()

set(CURL_VERSION ${PC_CURL_VERSION})
set(CURL_DEFINITIONS ${PC_CURL_CFLAGS_OTHER})

if(NOT CURL_INCLUDE_DIR OR NOT CURL_LIBRARIES)
    find_path(CURL_INCLUDE_DIR
        NAMES curl/curl.h
        HINTS $ENV{CURLDIR}/include
    )

    find_library(CURL_LIBRARIES
        NAMES curl
        HINTS $ENV{CURLDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Curl
    VERSION_VAR CURL_VERSION
    REQUIRED_VARS CURL_LIBRARIES CURL_INCLUDE_DIR
)

mark_as_advanced(CURL_INCLUDE_DIR CURL_LIBRARIES)
