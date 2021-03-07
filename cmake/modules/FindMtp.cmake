# Try to find the libmtp library, once done this will define:
#
#  MTP_FOUND - system has libmtp
#  MTP_INCLUDE_DIR - the libmtp include directory
#  MTP_LIBRARIES - the libraries needed to use libmtp
#  MTP_DEFINITIONS - compiler switches required for using libmtp
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_MTP QUIET libmtp)

    set(MTP_INCLUDE_DIR ${PC_MTP_INCLUDE_DIRS})
    set(MTP_LIBRARIES ${PC_MTP_LIBRARIES})
endif()

set(MTP_VERSION ${PC_MTP_VERSION})
set(MTP_DEFINITIONS ${PC_MTP_CFLAGS_OTHER})

if(NOT MTP_INCLUDE_DIR OR NOT MTP_LIBRARIES)
    find_path(MTP_INCLUDE_DIR
        NAMES libmtp.h
        HINTS $ENV{MTPDIR}/include
    )

    find_library(MTP_LIBRARIES
        NAMES mtp
        HINTS $ENV{MTPDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mtp
    VERSION_VAR MTP_VERSION
    REQUIRED_VARS MTP_LIBRARIES MTP_INCLUDE_DIR
)

mark_as_advanced(MTP_INCLUDE_DIR MTP_LIBRARIES)
