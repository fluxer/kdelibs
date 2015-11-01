# - Try to find libwebp
# Once done this will define
#
#  WEBP_FOUND - system has libwebp
#  WEBP_INCLUDES - the libwebp include directory
#  WEBP_LIBRARIES - The libraries needed to use libwebp
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if(WEBP_INCLUDES AND WEBP_LIBRARIES)
    set(WEBP_FIND_QUIETLY TRUE)
endif(WEBP_INCLUDES AND WEBP_LIBRARIES)

find_path(WEBP_INCLUDES
    NAMES
    encode.h
    decode.h
    PATH_SUFFIXES webp
    HINTS
    $ENV{WEBPDIR}/include
    /usr/include
    /usr/local/include
    ${INCLUDE_INSTALL_DIR}
)

find_library(WEBP_LIBRARIES
    webp
    HINTS
    $ENV{WEBPDIR}/lib
    /usr/lib
    /usr/local/lib
    ${LIB_INSTALL_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WEBP DEFAULT_MSG WEBP_INCLUDES WEBP_LIBRARIES)

mark_as_advanced(WEBP_INCLUDES WEBP_LIBRARIES)
