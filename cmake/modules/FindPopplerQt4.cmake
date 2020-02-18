# - Try to find the Qt4 binding of the Poppler library
#
# Once done this will define
#
#  POPPLER_QT4_FOUND - system has poppler-qt4
#  POPPLER_QT4_INCLUDE_DIR - the poppler-qt4 include directory
#  POPPLER_QT4_LIBRARIES - Link these to use poppler-qt4
#  POPPLER_QT4_DEFINITIONS - Compiler switches required for using poppler-qt4
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(POPPLER_QT4_NAMES poppler-katie poppler-qt4)

if(NOT WIN32)
    find_package(PkgConfig)
    foreach(name ${POPPLER_QT4_NAMES})
        if(NOT PC_POPPLER_QT4_FOUND)
            pkg_check_modules(PC_POPPLER_QT4 QUIET ${name})

            set(POPPLER_QT4_INCLUDE_DIR ${PC_POPPLER_QT4_INCLUDE_DIRS})
            set(POPPLER_QT4_LIBRARIES ${PC_POPPLER_QT4_LIBRARIES})
        endif()
    endforeach()
endif()

set(POPPLER_QT4_VERSION ${PC_POPPLER_QT4_VERSION})
set(POPPLER_QT4_DEFINITIONS ${PC_POPPLER_QT4_CFLAGS_OTHER})

if(NOT POPPLER_QT4_INCLUDE_DIR OR NOT POPPLER_QT4_LIBRARIES)
    find_path(POPPLER_QT4_INCLUDE_DIR
        NAMES poppler-katie.h poppler-qt4.h
        PATH_SUFFIXES poppler/katie poppler/qt4 poppler
        HINTS $ENV{POPPLERQT4DIR}/include
    )

    find_library(POPPLER_QT4_LIBRARIES
        NAMES ${POPPLER_QT4_NAMES}
        HINTS $ENV{POPPLERQT4DIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PopplerQt4
    VERSION_VAR POPPLER_QT4_VERSION
    REQUIRED_VARS POPPLER_QT4_INCLUDE_DIR POPPLER_QT4_LIBRARIES
)

# for compatibility
set(POPPLER_QT4_FOUND ${POPPLERQT4_FOUND})

mark_as_advanced(POPPLER_QT4_INCLUDE_DIR POPPLER_QT4_LIBRARIES)
