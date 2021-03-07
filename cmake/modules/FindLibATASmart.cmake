# Try to find ATA library, once done this will define:
#
#  LIBATASMART_FOUND - system has ATA library
#  LIBATASMART_INCLUDES - the ATA library include directory
#  LIBATASMART_LIBRARIES - the libraries needed to use ATA library
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBATASMART QUIET libatasmart)

    set(LIBATASMART_INCLUDES ${PC_LIBATASMART_INCLUDE_DIRS})
    set(LIBATASMART_LIBRARIES ${PC_LIBATASMART_LIBRARIES})
endif()

set(LIBATASMART_VERSION ${PC_LIBATASMART_VERSION})

if(NOT LIBATASMART_INCLUDES OR NOT LIBATASMART_LIBRARIES)
    find_path(LIBATASMART_INCLUDES
        NAMES atasmart.h
        HINTS $ENV{LIBATASMARTDIR}/include
    )

    find_library(LIBATASMART_LIBRARIES
        NAMES atasmart
        HINTS $ENV{LIBATASMARTDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibATASmart
    VERSION_VAR LIBATASMART_VERSION
    REQUIRED_VARS LIBATASMART_LIBRARIES LIBATASMART_INCLUDES
)

mark_as_advanced(LIBATASMART_INCLUDES LIBATASMART_LIBRARIES)
