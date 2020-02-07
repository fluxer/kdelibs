# - Try to find the Qt4 binding of the Poppler library
# Once done this will define
#
#  POPPLER_QT4_FOUND - system has poppler-qt4
#  POPPLER_QT4_INCLUDE_DIR - the poppler-qt4 include directory
#  POPPLER_QT4_LIBRARIES - Link these to use poppler-qt4
#  POPPLER_QT4_DEFINITIONS - Compiler switches required for using poppler-qt4
#

# Copyright (c) 2006, Wilfried Huss, <wilfried.huss@gmx.at>
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(POPPLER_QT4_NAMES poppler-katie poppler-qt4)


find_package(PkgConfig)
foreach(name ${POPPLER_QT4_NAMES})
    if(NOT PC_POPPLER_QT4_FOUND)
        pkg_check_modules(PC_POPPLER_QT4 QUIET ${name})
    endif()
endforeach()

set(POPPLER_QT4_DEFINITIONS ${PC_POPPLER_QT4_CFLAGS_OTHER})

find_path(POPPLER_QT4_INCLUDE_DIR
    NAMES poppler-katie.h poppler-qt4.h
    HINTS ${PC_POPPLER_QT4_INCLUDEDIR}
    PATH_SUFFIXES poppler/katie poppler/qt4 poppler
)

find_library(POPPLER_QT4_LIBRARIES
    NAMES ${POPPLER_QT4_NAMES}
    HINTS ${PC_POPPLER_QT4_LIBDIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PopplerQt4
    REQUIRED_VARS POPPLER_QT4_INCLUDE_DIR POPPLER_QT4_LIBRARIES
    VERSION_VAR PC_POPPLER_QT4_VERSION
)

# for compatibility:
set(POPPLER_QT4_FOUND ${POPPLERQT4_FOUND})
  
mark_as_advanced(POPPLER_QT4_INCLUDE_DIR POPPLER_QT4_LIBRARIES)
