# Try to find LibSSH, once done this will define:
#
#  LIBSSH_FOUND - system has LibSSH
#  LIBSSH_INCLUDE_DIRS - the LibSSH include directory
#  LIBSSH_LIBRARIES - the libraries needed to use LibSSH
#  LIBSSH_DEFINITIONS - compiler switches required for using LibSSH
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBSSH QUIET libssh)

    set(LIBSSH_INCLUDE_DIR ${PC_LIBSSH_INCLUDE_DIRS})
    set(LIBSSH_LIBRARIES ${PC_LIBSSH_LIBRARIES})
endif()

set(LIBSSH_VERSION ${PC_LIBSSH_VERSION})
set(LIBSSH_DEFINITIONS ${PC_LIBSSH_CFLAGS_OTHER})

if(NOT LIBSSH_INCLUDE_DIR OR NOT LIBSSH_LIBRARIES)
    find_path(LIBSSH_INCLUDE_DIR
        NAMES libssh/libssh.h
        HINTS $ENV{LIBSSHDIR}/include
    )

    find_library(LIBSSH_LIBRARIES
        NAMES ssh
        HINTS $ENV{LIBSSHDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibSSH
    VERSION_VAR LIBSSH_VERSION
    REQUIRED_VARS LIBSSH_LIBRARIES LIBSSH_INCLUDE_DIR
)

mark_as_advanced(LIBSSH_INCLUDE_DIR LIBSSH_LIBRARIES)
