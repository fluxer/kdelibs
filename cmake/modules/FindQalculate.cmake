# Try to find Qalculate, once done this will define:
#
#  QALCULATE_FOUND - system has Qalculate
#  QALCULATE_INCLUDE_DIR - the Qalculate include directory
#  QALCULATE_LIBRARIES - the Qalculate libraries
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_QALCULATE QUIET libqalculate)

set(QALCULATE_INCLUDE_DIR ${PC_QALCULATE_INCLUDE_DIRS})
set(QALCULATE_LIBRARIES ${PC_QALCULATE_LIBRARIES})
set(QALCULATE_VERSION ${PC_QALCULATE_VERSION})

if(NOT QALCULATE_INCLUDE_DIR OR NOT QALCULATE_LIBRARIES)
    find_path(QALCULATE_INCLUDE_DIR
        NAMES qalculate.h
        PATH_SUFFIXES libqalculate
        HINTS $ENV{QALCULATEDIR}/include
    )

    find_library(QALCULATE_LIBRARIES
        NAMES qalculate
        HINTS $ENV{QALCULATEDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qalculate
    VERSION_VAR QALCULATE_VERSION
    REQUIRED_VARS QALCULATE_LIBRARIES QALCULATE_INCLUDE_DIR
)

mark_as_advanced(QALCULATE_INCLUDE_DIR QALCULATE_LIBRARIES)
