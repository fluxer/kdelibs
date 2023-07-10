# Try to find libgit2 library, once done this will define:
#
#  LIBGIT2_FOUND - system has libgit2
#  LIBGIT2_INCLUDE_DIR - the libgit2 include directory
#  LIBGIT2_LIBRARIES - the libraries needed to use libgit2
#
# Copyright (c) 2023 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBGIT2 QUIET libgit2)

set(LIBGIT2_INCLUDE_DIR ${PC_LIBGIT2_INCLUDE_DIRS})
set(LIBGIT2_LIBRARIES ${PC_LIBGIT2_LIBRARIES})
set(LIBGIT2_VERSION ${PC_LIBGIT2_VERSION})

if(NOT LIBGIT2_INCLUDE_DIR OR NOT LIBGIT2_LIBRARIES)
    find_path(LIBGIT2_INCLUDE_DIR
        NAMES git2.h
        HINTS $ENV{LIBGIT2DIR}/include
    )

    find_library(LIBGIT2_LIBRARIES
        NAMES git2
        HINTS $ENV{LIBGIT2DIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibGit2
    VERSION_VAR LIBGIT2_VERSION
    REQUIRED_VARS LIBGIT2_LIBRARIES LIBGIT2_INCLUDE_DIR
)

mark_as_advanced(LIBGIT2_INCLUDE_DIR LIBGIT2_LIBRARIES)
