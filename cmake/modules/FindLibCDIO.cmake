# Try to find libcdio, once done this will define:
#
#  LIBCDIO_FOUND - system has libcdio
#  LIBCDIO_INCLUDES - the libcdio include directory
#  LIBCDIO_LIBRARIES - the libraries needed to use libcdio
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBCDIO QUIET libcdio)

set(LIBCDIO_INCLUDES ${PC_LIBCDIO_INCLUDE_DIRS})
set(LIBCDIO_LIBRARIES ${PC_LIBCDIO_LIBRARIES})
set(LIBCDIO_VERSION ${PC_LIBCDIO_VERSION})

if(NOT LIBCDIO_INCLUDES OR NOT LIBCDIO_LIBRARIES)
    find_path(LIBCDIO_INCLUDES
        NAMES cdio/cdio.h
        HINTS $ENV{LIBCDIODIR}/include
    )

    find_library(LIBCDIO_LIBRARIES
        NAMES cdio
        HINTS $ENV{LIBCDIODIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibCDIO
    VERSION_VAR LIBCDIO_VERSION
    REQUIRED_VARS LIBCDIO_LIBRARIES LIBCDIO_INCLUDES
)

mark_as_advanced(LIBCDIO_INCLUDES LIBCDIO_LIBRARIES)
