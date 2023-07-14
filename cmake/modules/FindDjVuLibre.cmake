# Try to find the DjVuLibre library, once done this will define:
#
#  DJVULIBRE_FOUND - system has the DjVuLibre library
#  DJVULIBRE_INCLUDE_DIR - the DjVuLibre include directory
#  DJVULIBRE_LIBRARY - Link this to use the DjVuLibre library
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_DJVULIBRE QUIET ddjvuapi)

set(DJVULIBRE_VERSION ${PC_DJVULIBRE_VERSION})

if(NOT DJVULIBRE_INCLUDE_DIR OR NOT DJVULIBRE_LIBRARY)
    find_path(DJVULIBRE_INCLUDE_DIR
        NAMES libdjvu/ddjvuapi.h
        HINTS $ENV{DJVULIBREDIR}/include
    )

    find_library(DJVULIBRE_LIBRARY
        NAMES djvulibre
        HINTS $ENV{DJVULIBREDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DjVuLibre
    VERSION_VAR DJVULIBRE_VERSION
    REQUIRED_VARS DJVULIBRE_LIBRARY DJVULIBRE_INCLUDE_DIR
)

mark_as_advanced(DJVULIBRE_INCLUDE_DIR DJVULIBRE_LIBRARY)
