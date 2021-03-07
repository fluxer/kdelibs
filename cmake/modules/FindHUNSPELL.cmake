# Try to find HUNSPELL, once done this will define:
#
#  HUNSPELL_FOUND - system has HUNSPELL
#  HUNSPELL_INCLUDE_DIR - the HUNSPELL include directory
#  HUNSPELL_LIBRARIES - the libraries needed to use HUNSPELL
#  HUNSPELL_DEFINITIONS - compiler switches required for using HUNSPELL
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_HUNSPELL QUIET hunspell)

    set(HUNSPELL_INCLUDE_DIR ${PC_HUNSPELL_INCLUDE_DIRS})
    set(HUNSPELL_LIBRARIES ${PC_HUNSPELL_LIBRARIES})
endif()

set(HUNSPELL_VERSION ${PC_HUNSPELL_VERSION})
set(HUNSPELL_DEFINITIONS ${PC_HUNSPELL_CFLAGS_OTHER})

if(NOT HUNSPELL_INCLUDE_DIR OR NOT HUNSPELL_LIBRARIES)
    find_path(HUNSPELL_INCLUDE_DIR
        NAMES hunspell/hunspell.hxx
        HINTS $ENV{HUNSPELLDIR}/include
    )

    find_library(HUNSPELL_LIBRARIES
        NAMES hunspell hunspell-1.7 hunspell-1.6 hunspell-1.5 hunspell-1.4 hunspell-1.3 hunspell-1.2
        HINTS $ENV{HUNSPELLDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HUNSPELL
    VERSION_VAR HUNSPELL_VERSION
    REQUIRED_VARS HUNSPELL_LIBRARIES HUNSPELL_INCLUDE_DIR
)

mark_as_advanced(HUNSPELL_INCLUDE_DIR HUNSPELL_LIBRARIES)
