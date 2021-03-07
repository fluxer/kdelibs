# Try to find the Attica library, once done this will define:
#
#  LIBATTICA_FOUND - system has Attica
#  LIBATTICA_LIBRARIES - the Attica include directory
#  LIBATTICA_INCLUDE_DIR - the libraries needed to use Attica
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(LIBATTICA_NAMES libattica-katie libattica)

if(NOT WIN32)
    include(FindPkgConfig)
    foreach(name ${LIBATTICA_NAMES})
        if(NOT PC_LIBATTICA_FOUND)
            pkg_check_modules(PC_LIBATTICA QUIET ${name})

            set(LIBATTICA_INCLUDE_DIR ${PC_LIBATTICA_INCLUDE_DIRS})
            set(LIBATTICA_LIBRARIES ${PC_LIBATTICA_LIBRARIES})
        endif()
    endforeach()
endif()

set(LIBATTICA_VERSION ${PC_LIBATTICA_VERSION})

if(NOT LIBATTICA_INCLUDE_DIR OR NOT LIBATTICA_LIBRARIES)
    find_path(LIBATTICA_INCLUDE_DIR
        NAMES attica/provider.h
        PATH_SUFFIXES attica-katie attica
        HINTS $ENV{LIBATTICADIR}/include
    )

    find_library(LIBATTICA_LIBRARIES
        NAMES attica-katie attica
        HINTS $ENV{LIBATTICADIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibAttica
    VERSION_VAR LIBATTICA_VERSION
    REQUIRED_VARS LIBATTICA_LIBRARIES LIBATTICA_INCLUDE_DIR
)

mark_as_advanced(LIBATTICA_INCLUDE_DIR LIBATTICA_LIBRARIES)
