# Try to find Avahi library, once done this will define:
#
#  AVAHI_FOUND - system has Avahi
#  AVAHI_INCLUDE_DIR - the Avahi include directory
#  AVAHI_LIBRARIES - the libraries needed to use Avahi
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_AVAHI QUIET avahi-client)

    set(AVAHI_INCLUDE_DIR ${PC_AVAHI_INCLUDE_DIRS})
    set(AVAHI_LIBRARIES ${PC_AVAHI_LIBRARIES})
endif()

set(AVAHI_VERSION ${PC_AVAHI_VERSION})

if(NOT AVAHI_INCLUDE_DIR OR NOT AVAHI_LIBRARIES)
    find_path(AVAHI_INCLUDE_DIR
        NAMES avahi-client/client.h
        HINTS $ENV{AVAHIDIR}/include
    )

    find_library(AVAHI_LIBRARIES
        NAMES avahi-client
        HINTS $ENV{AVAHIDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Avahi
    VERSION_VAR AVAHI_VERSION
    REQUIRED_VARS AVAHI_LIBRARIES AVAHI_INCLUDE_DIR
)

mark_as_advanced(AVAHI_INCLUDE_DIR AVAHI_LIBRARIES)
