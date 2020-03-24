# - Try to find the libgcrypt library
#
# Once done this will define
#
#  LIBGCRYPT_FOUND - system has libgcrypt
#  LIBGCRYPT_INCLUDE_DIR - the libgcrypt include directory
#  LIBGCRYPT_LIBRARIES - the libraries needed to use libgcrypt
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(LIBGCRYPT_INCLUDE_DIR
    NAMES gcrypt.h
    PATH_SUFFIXES libgcrypt
    HINTS $ENV{LIBGCRYPTDIR}/include
)

find_library(LIBGCRYPT_LIBRARIES
    NAMES gcrypt
    HINTS $ENV{LIBGCRYPTDIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibGcrypt
    REQUIRED_VARS LIBGCRYPT_LIBRARIES LIBGCRYPT_INCLUDE_DIR
)