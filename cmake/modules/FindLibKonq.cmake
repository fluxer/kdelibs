# Try to find konqueror library, once done this will define:
#
#  LIBKONQ_FOUND - system has konqueror library
#  LIBKONQ_INCLUDE_DIR - the konqueror library include directory
#  LIBKONQ_LIBRARY - the konqueror library
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(LIBKONQ_INCLUDE_DIR
    NAMES konq_popupmenuplugin.h
    HINTS $ENV{LIBKONQDIR}/include
)

find_library(LIBKONQ_LIBRARY
    NAMES konq
    HINTS $ENV{LIBKONQDIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibKonq
    REQUIRED_VARS LIBKONQ_LIBRARY LIBKONQ_INCLUDE_DIR
)