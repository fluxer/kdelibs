# - Try to find the Exiv2 library
#
#  EXIV2_MIN_VERSION - You can set this variable to the minimum version you need
#                      before doing FIND_PACKAGE(Exiv2). The default is 0.12.
#
# Once done this will define
#
#  EXIV2_FOUND - system has libexiv2
#  EXIV2_INCLUDE_DIR - the libexiv2 include directory
#  EXIV2_LIBRARIES - Link these to use libexiv2
#  EXIV2_DEFINITIONS - Compiler switches required for using libexiv2
#
# The minimum required version of Exiv2 can be specified using the
# standard syntax, e.g. find_package(Exiv2 0.17)
#
# For compatibility, also the variable EXIV2_MIN_VERSION can be set to the minimum version
# you need before doing FIND_PACKAGE(Exiv2). The default is 0.12.

# Copyright (c) 2010, Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2008, Gilles Caulier, <caulier.gilles@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Support EXIV2_MIN_VERSION for compatibility:
if(NOT Exiv2_FIND_VERSION)
  set(Exiv2_FIND_VERSION "${EXIV2_MIN_VERSION}")
endif(NOT Exiv2_FIND_VERSION)

# the minimum version of exiv2 we require
if(NOT Exiv2_FIND_VERSION)
  set(Exiv2_FIND_VERSION "0.12")
endif(NOT Exiv2_FIND_VERSION)

if(NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    pkg_check_modules(PC_EXIV2 QUIET exiv2)
    set(EXIV2_DEFINITIONS ${PC_EXIV2_CFLAGS_OTHER})
    set(EXIV2_VERSION ${PC_EXIV2_VERSION})
endif()


find_path(EXIV2_INCLUDE_DIR
    NAMES
    exiv2/exif.hpp
    HINTS
    ${PC_EXIV2_INCLUDEDIR}
    ${PC_EXIV2_INCLUDE_DIRS}
)

find_library(EXIV2_LIBRARY
    NAMES
    exiv2
    libexiv2
    HINTS
    ${PC_EXIV2_LIBDIR}
    ${PC_EXIV2_LIBRARY_DIRS}
)

set(EXIV2_LIBRARIES "${EXIV2_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Exiv2  REQUIRED_VARS  EXIV2_LIBRARY EXIV2_INCLUDE_DIR
                                         VERSION_VAR  EXIV2_VERSION)

mark_as_advanced(EXIV2_INCLUDE_DIR EXIV2_LIBRARY)

