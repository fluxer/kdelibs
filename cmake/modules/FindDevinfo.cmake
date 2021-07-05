# Try to find devinfo, once done this will define:
#
#  DEVINFO_FOUND - system has devinfo
#  DEVINFO_INCLUDES - the devinfo include directory
#  DEVINFO_LIBRARIES - the libraries needed to use devinfo
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(DEVINFO_INCLUDES
    NAMES devinfo.h
    HINTS $ENV{DEVINFODIR}/include
)

find_library(DEVINFO_LIBRARIES
    NAMES devinfo
    HINTS $ENV{DEVINFODIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Devinfo
    REQUIRED_VARS DEVINFO_LIBRARIES DEVINFO_INCLUDES
)

mark_as_advanced(DEVINFO_INCLUDES DEVINFO_LIBRARIES)
