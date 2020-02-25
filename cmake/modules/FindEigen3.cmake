# - Try to find Eigen3
#
# Once done this will define
#
#  EIGEN3_FOUND - system has Eigen3
#  EIGEN3_INCLUDE_DIR - the Eigen3 include directory
#
# Copyright (c) 2020, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_EIGEN3 QUIET eigen3)

    set(EIGEN3_INCLUDE_DIR ${PC_EIGEN3_INCLUDE_DIRS})
endif()

set(EIGEN3_VERSION ${PC_EIGEN3_VERSION})

if(NOT EIGEN3_INCLUDE_DIR)
    find_path(EIGEN3_INCLUDE_DIR
        NAMES signature_of_eigen3_matrix_library
        PATH_SUFFIXES eigen3 eigen
        HINTS $ENV{EIGEN3DIR}/include
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Eigen3
    VERSION_VAR EIGEN3_VERSION
    REQUIRED_VARS EIGEN3_INCLUDE_DIR
)

mark_as_advanced(EIGEN3_INCLUDE_DIR)
