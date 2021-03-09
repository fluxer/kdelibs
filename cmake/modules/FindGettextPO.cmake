# Try to find GettextPO, once done this will define:
#
#  GETTEXTPO_FOUND - System has GettextPO
#  GETTEXTPO_INCLUDE_DIR - The GettextPO include directory
#  GETTEXTPO_LIBRARY - The library needed to use GettextPO
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(GETTEXTPO_INCLUDE_DIR
    NAMES gettext-po.h
    HINTS $ENV{GETTEXTPODIR}/include
)

find_library(GETTEXTPO_LIBRARY
    NAMES gettextpo
    HINTS $ENV{GETTEXTPODIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GettextPO
    REQUIRED_VARS GETTEXTPO_LIBRARY GETTEXTPO_INCLUDE_DIR
)

mark_as_advanced(GETTEXTPO_INCLUDE_DIR GETTEXTPO_LIBRARY)
