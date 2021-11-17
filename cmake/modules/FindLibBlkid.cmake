# Try to find blkid library, once done this will define:
#
#  LIBBLKID_FOUND - system has blkid library
#  LIBBLKID_INCLUDES - the blkid library include directory
#  LIBBLKID_LIBRARIES - the libraries needed to use blkid library
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBBLKID QUIET blkid)

    set(LIBBLKID_INCLUDES ${PC_LIBBLKID_INCLUDE_DIRS})
    set(LIBBLKID_LIBRARIES ${PC_LIBBLKID_LIBRARIES})
endif()

set(LIBBLKID_VERSION ${PC_LIBBLKID_VERSION})

if(NOT LIBBLKID_INCLUDES OR NOT LIBBLKID_LIBRARIES)
    find_path(LIBBLKID_INCLUDES
        NAMES blkid/blkid.h
        HINTS $ENV{LIBBLKIDDIR}/include
    )

    find_library(LIBBLKID_LIBRARIES
        NAMES blkid
        HINTS $ENV{LIBBLKIDDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibBlkid
    VERSION_VAR LIBBLKID_VERSION
    REQUIRED_VARS LIBBLKID_LIBRARIES LIBBLKID_INCLUDES
)

mark_as_advanced(LIBBLKID_INCLUDES LIBBLKID_LIBRARIES)
