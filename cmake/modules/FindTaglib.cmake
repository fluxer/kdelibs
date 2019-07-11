# - Try to find the Taglib library
# Once done this will define
#
#  TAGLIB_FOUND - system has the taglib library
#  TAGLIB_CFLAGS - the taglib cflags
#  TAGLIB_LIBRARIES - The libraries needed to use taglib

# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT TAGLIB_MIN_VERSION)
  set(TAGLIB_MIN_VERSION "1.4")
endif(NOT TAGLIB_MIN_VERSION)

#reset vars
set(TAGLIB_LIBRARIES)
set(TAGLIB_CFLAGS)

include(FindPackageHandleStandardArgs)

find_path(TAGLIB_INCLUDES
    NAMES
    tag.h
    PATH_SUFFIXES taglib
    PATHS
    ${KDE4_INCLUDE_DIR}
    ${INCLUDE_INSTALL_DIR}
)

find_library(TAGLIB_LIBRARIES
    NAMES tag
    PATHS
    ${KDE4_LIB_DIR}
    ${LIB_INSTALL_DIR}
)

find_package_handle_standard_args(Taglib DEFAULT_MSG
    TAGLIB_INCLUDES TAGLIB_LIBRARIES
)

if(TAGLIB_FOUND)
  if(NOT Taglib_FIND_QUIETLY AND TAGLIB_LIBRARIES)
    message(STATUS "Taglib found: ${TAGLIB_LIBRARIES}")
  endif()
else(TAGLIB_FOUND)
  if(Taglib_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find Taglib")
  endif()
endif(TAGLIB_FOUND)

