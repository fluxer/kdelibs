# - Try to find Sqlite
#
# Once done this will define
#
#  SQLITE_FOUND - system has Sqlite
#  SQLITE_INCLUDE_DIR - the Sqlite include directory
#  SQLITE_LIBRARIES - the libraries needed to use Sqlite
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_SQLITE QUIET sqlite3)

    set(SQLITE_INCLUDE_DIR ${PC_SQLITE_INCLUDE_DIRS})
    set(SQLITE_LIBRARIES ${PC_SQLITE_LIBRARIES})
endif()

set(SQLITE_VERSION ${PC_SQLITE_VERSION})

if(NOT SQLITE_INCLUDE_DIR OR NOT SQLITE_LIBRARIES)
    find_path(SQLITE_INCLUDE_DIR
        NAMES sqlite3.h
        HINTS $ENV{SQLITEDIR}/include
    )

    find_library(SQLITE_LIBRARIES
        NAMES sqlite3
        HINTS $ENV{SQLITEDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sqlite
    VERSION_VAR SQLITE_VERSION
    REQUIRED_VARS SQLITE_LIBRARIES SQLITE_INCLUDE_DIR
)

mark_as_advanced(SQLITE_INCLUDE_DIR SQLITE_LIBRARIES)
