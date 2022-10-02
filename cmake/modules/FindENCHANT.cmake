# Try to find the Enchant spell checker, once done this will define:
#
#  ENCHANT_FOUND - system has ENCHANT
#  ENCHANT_INCLUDE_DIR - the ENCHANT include directory
#  ENCHANT_LIBRARIES - the libraries needed to use ENCHANT
#  ENCHANT_DEFINITIONS - compiler switches required for using ENCHANT
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(ENCHANT_NAMES enchant-2 enchant)

find_package(PkgConfig REQUIRED)
foreach(name ${ENCHANT_NAMES})
    if(NOT PC_ENCHANT_FOUND)
        pkg_check_modules(PC_ENCHANT QUIET ${name})

        set(ENCHANT_INCLUDE_DIR ${PC_ENCHANT_INCLUDE_DIRS})
        set(ENCHANT_LIBRARIES ${PC_ENCHANT_LIBRARIES})
    endif()
endforeach()

set(ENCHANT_VERSION ${PC_ENCHANT_VERSION})
set(ENCHANT_DEFINITIONS ${PC_ENCHANT_CFLAGS_OTHER})

if(NOT ENCHANT_INCLUDE_DIR OR NOT ENCHANT_LIBRARIES)
    find_path(ENCHANT_INCLUDE_DIR
        NAMES enchant++.h
        PATH_SUFFIXES ${ENCHANT_NAMES}
        HINTS $ENV{ENCHANTDIR}/include
    )

    find_library(ENCHANT_LIBRARIES
        NAMES ${ENCHANT_NAMES}
        HINTS $ENV{ENCHANTDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ENCHANT
    VERSION_VAR ENCHANT_VERSION
    REQUIRED_VARS ENCHANT_LIBRARIES ENCHANT_INCLUDE_DIR
)

mark_as_advanced(ENCHANT_INCLUDE_DIR ENCHANT_LIBRARIES)
