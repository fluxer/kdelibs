# Try to find DNSSD, once done this will define:
#
#  DNSSD_FOUND - system has DNSSD
#  DNSSD_INCLUDE_DIR - the DNSSD include directory
#  DNSSD_LIBRARIES - Link these to use dnssd
#  DNSSD_DEFINITIONS - Compiler switches required for using DNSSD
#
# need more test: look at into dnssd/configure.in.in

# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(CMakePushCheckState)

find_path(DNSSD_INCLUDE_DIR
    NAMES dns_sd.h
    HINTS /usr/include/avahi-compat-libdns_sd/
)

if(DNSSD_INCLUDE_DIR)
    find_library(DNSSD_LIBRARIES NAMES dns_sd)

    cmake_reset_check_state()
    set(CMAKE_REQUIRED_INCLUDES ${DNSSD_INCLUDE_DIR})
    set(CMAKE_REQUIRED_LIBRARIES ${DNSSD_LIBRARIES})
    CHECK_FUNCTION_EXISTS(DNSServiceRefDeallocate DNSSD_FUNCTION_FOUND)
    cmake_reset_check_state()

    if(DNSSD_INCLUDE_DIR AND DNSSD_LIBRARIES AND DNSSD_FUNCTION_FOUND)
        set(DNSSD_FOUND TRUE)
    endif()
endif()

if(DNSSD_FOUND)
    if (NOT DNSSD_FIND_QUIETLY)
        message(STATUS "Found DNSSD: ${DNSSD_LIBRARIES}")
    endif()
else()
    if(DNSSD_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find DNSSD")
    endif()
endif()

MARK_AS_ADVANCED(DNSSD_INCLUDE_DIR DNSSD_LIBRARIES)
