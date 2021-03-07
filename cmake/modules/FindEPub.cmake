# Try to find EPub library, once done this will define:
#
#  EPUB_FOUND - system has EPub library
#  EPUB_INCLUDE_DIR - the EPub library include directory
#  EPUB_LIBRARIES - the libraries needed to use EPub library
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(EPUB_INCLUDE_DIR
    NAMES epub.h
    HINTS $ENV{EPUBDIR}/include
)

find_library(EPUB_LIBRARIES
    NAMES epub
    HINTS $ENV{EPUBDIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EPub
    REQUIRED_VARS EPUB_LIBRARIES EPUB_INCLUDE_DIR
)
