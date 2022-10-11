# Try to find OpenJPEG library, once done this will define:
#
#  OPENJPEG_FOUND - system has OpenJPEG
#  OPENJPEG_INCLUDE_DIR - the OpenJPEG include directory
#  OPENJPEG_LIBRARIES - the libraries needed to use OpenJPEG
#  OPENJPEG_DEFINITIONS - compiler switches required for using OpenJPEG
#
# Copyright (c) 2022 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_OPENJPEG QUIET libopenjp2)

set(OPENJPEG_INCLUDE_DIR ${PC_OPENJPEG_INCLUDE_DIRS})
set(OPENJPEG_LIBRARIES ${PC_OPENJPEG_LIBRARIES})
set(OPENJPEG_VERSION ${PC_OPENJPEG_VERSION})
set(OPENJPEG_DEFINITIONS ${PC_OPENJPEG_CFLAGS_OTHER})

if(NOT OPENJPEG_INCLUDE_DIR OR NOT OPENJPEG_LIBRARIES)
    find_path(OPENJPEG_INCLUDE_DIR
        NAMES openjpeg.h
        PATH_SUFFIXES openjpeg
        HINTS $ENV{OPENJPEGDIR}/include
    )

    find_library(OPENJPEG_LIBRARIES
        NAMES openjp2
        HINTS $ENV{OPENJPEGDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenJPEG
    VERSION_VAR OPENJPEG_VERSION
    REQUIRED_VARS OPENJPEG_LIBRARIES OPENJPEG_INCLUDE_DIR
)

mark_as_advanced(OPENJPEG_INCLUDE_DIR OPENJPEG_LIBRARIES)
