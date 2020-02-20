# - Try to find the pciutils directory library
#
# Once done this will define
#
#  PCIUTILS_FOUND - system has PCIUtils
#  PCIUTILS_INCLUDE_DIR - the PCIUTILS include directory
#  PCIUTILS_LIBRARIES - the libraries needed to use PCIUtils
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_PCIUTILS QUIET libpci)

    set(PCIUTILS_INCLUDE_DIR ${PC_PCIUTILS_INCLUDE_DIRS})
    set(PCIUTILS_LIBRARIES ${PC_PCIUTILS_LIBRARIES})
endif()

set(PCIUTILS_VERSION ${PC_PCIUTILS_VERSION})

if(NOT PCIUTILS_INCLUDE_DIR OR NOT PCIUTILS_LIBRARIES)
    find_path(PCIUTILS_INCLUDE_DIR
        NAMES pci/pci.h
        HINTS $ENV{PCIUTILSDIR}/include
    )

    find_library(PCIUTILS_LIBRARIES
        NAMES pci
        HINTS $ENV{PCIUTILSDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCIUTILS
    VERSION_VAR PCIUTILS_VERSION
    REQUIRED_VARS PCIUTILS_LIBRARIES PCIUTILS_INCLUDE_DIR
)

mark_as_advanced(PCIUTILS_INCLUDE_DIR PCIUTILS_LIBRARIES)
