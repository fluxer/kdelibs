# - Try to find QCA2 (Qt Cryptography Architecture 2)
#
# Once done this will define
#
#  QCA2_FOUND - system has QCA2
#  QCA2_INCLUDE_DIR - the QCA2 include directory
#  QCA2_LIBRARIES - the libraries needed to use QCA2
#  QCA2_DEFINITIONS - Compiler switches required for using QCA2
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(QCA2_NAMES qca2-katie qca2)

if(NOT WIN32)
    find_package(PkgConfig)
    foreach(name ${QCA2_NAMES})
        if(NOT PC_QCA2_FOUND)
            pkg_check_modules(PC_QCA2 QUIET ${name})

            set(QCA2_INCLUDE_DIR ${PC_QCA2_INCLUDE_DIRS})
            set(QCA2_LIBRARIES ${PC_QCA2_LIBRARIES})
        endif()
    endforeach()
endif()

set(QCA2_DEFINITIONS ${PC_QCA2_CFLAGS_OTHER})

if(NOT QCA2_INCLUDE_DIR OR NOT QCA2_LIBRARIES)
    find_path(QCA2_INCLUDE_DIR
        NAMES QtCrypto
        PATH_SUFFIXES Qca-katie/QtCrypto Qca/QtCrypto QtCrypto
        HINTS $ENV{QCA2DIR}/include
    )

    find_library(QCA2_LIBRARIES
        NAMES qca-katie qca
        HINTS $ENV{QCA2DIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QCA2
    VERSION_VAR PC_QCA2_VERSION
    REQUIRED_VARS QCA2_LIBRARIES QCA2_INCLUDE_DIR
)

mark_as_advanced(QCA2_INCLUDE_DIR QCA2_LIBRARIES)
