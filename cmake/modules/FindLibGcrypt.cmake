# Try to find the libgcrypt library, once done this will define:
#
#  LIBGCRYPT_FOUND - system has libgcrypt
#  LIBGCRYPT_INCLUDE_DIR - the libgcrypt include directory
#  LIBGCRYPT_LIBRARIES - the libraries needed to use libgcrypt
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBGCRYPT QUIET libgcrypt)

    set(LIBGCRYPT_INCLUDE_DIR ${PC_LIBGCRYPT_INCLUDE_DIRS})
    set(LIBGCRYPT_LIBRARIES ${PC_LIBGCRYPT_LIBRARIES})
endif()

set(LIBGCRYPT_VERSION ${PC_LIBGCRYPT_VERSION})

if(NOT LIBGCRYPT_INCLUDE_DIR OR NOT LIBGCRYPT_LIBRARIES)
    find_path(LIBGCRYPT_INCLUDE_DIR
        NAMES gcrypt.h
        PATH_SUFFIXES libgcrypt
        HINTS $ENV{LIBGCRYPTDIR}/include
    )

    find_library(LIBGCRYPT_LIBRARIES
        NAMES gcrypt
        HINTS $ENV{LIBGCRYPTDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibGcrypt
    VERSION_VAR LIBGCRYPT_VERSION
    REQUIRED_VARS LIBGCRYPT_LIBRARIES LIBGCRYPT_INCLUDE_DIR
)
