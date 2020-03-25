# - Try to find Avahi
#
# Once done this will define
#
#  AVAHI_FOUND - system has Avahi
#  AVAHI_INCLUDE_DIR - the Avahi include directory
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(AVAHI_INCLUDE_DIR
    NAMES avahi-common/defs.h
    HINTS $ENV{AVAHIDIR}/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Avahi
    REQUIRED_VARS AVAHI_INCLUDE_DIR
)
