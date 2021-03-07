# Try to find the Taglib, once done this will define:
#
#  TAGLIB_FOUND - system has Taglib
#  TAGLIB_LIBRARIES - the libraries needed to use Taglib
#  TAGLIB_CFLAGS - the Taglib C flags
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_TAGLIB QUIET taglib)

    set(TAGLIB_INCLUDE_DIR ${PC_TAGLIB_INCLUDE_DIRS})
    set(TAGLIB_LIBRARY ${PC_TAGLIB_LIBRARIES})
endif()

set(TAGLIB_VERSION ${PC_TAGLIB_VERSION})
set(TAGLIB_CFLAGS ${PC_TAGLIB_CFLAGS_OTHER})

if(NOT TAGLIB_INCLUDES OR NOT TAGLIB_LIBRARIES)
    find_path(TAGLIB_INCLUDES
        NAMES tag.h
        PATH_SUFFIXES taglib
        HINTS $ENV{TAGLIBDIR}/include
    )

    find_library(TAGLIB_LIBRARIES
        NAMES tag
        HINTS $ENV{TAGLIBDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Taglib
    VERSION_VAR TAGLIB_VERSION
    REQUIRED_VARS TAGLIB_LIBRARIES TAGLIB_INCLUDES
)

mark_as_advanced(TAGLIB_INCLUDES TAGLIB_LIBRARIES)
