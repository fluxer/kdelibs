# cmake macro to test LIB NXCL

# Copyright (c) 2008, David Gross <gdavid.devel@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(CheckStructHasMember)
include(CMakePushCheckState)

IF (LIBNXCL_INCLUDE_DIR AND LIBNXCL_LIBRARIES)
    # Already in cache, be silent
    SET(LIBNBXCL_FIND_QUIETLY TRUE)
ENDIF (LIBNXCL_INCLUDE_DIR AND LIBNXCL_LIBRARIES)

FIND_PATH(LIBNXCL_INCLUDE_DIR nxcl/nxclientlib.h)
FIND_LIBRARY(LIBNXCL_LIBRARIES NAMES nxcl libnxcl)

IF (LIBNXCL_INCLUDE_DIR AND LIBNXCL_LIBRARIES)
    cmake_reset_check_state()
    SET(CMAKE_REQUIRED_LIBRARIES "${LIBNXCL_LIBRARIES}")
    SET(CMAKE_REQUIRED_INCLUDES "${LIBNXCL_INCLUDE_DIR}")
    CHECK_STRUCT_HAS_MEMBER(nxcl::NXClientLib "getNXSSHProcess()" nxcl/nxclientlib.h LIBNXCL_NXSSHPROCESS_FOUND)
    CHECK_STRUCT_HAS_MEMBER(nxcl::NXClientLib "getXID()" nxcl/nxclientlib.h LIBNXCL_XID_FOUND)
    cmake_reset_check_state()
ENDIF (LIBNXCL_INCLUDE_DIR AND LIBNXCL_LIBRARIES)

IF (LIBNXCL_NXSSHPROCESS_FOUND AND LIBNXCL_XID_FOUND)
    SET(LIBNXCL_FOUND TRUE)
    IF (NOT LIBNXCL_FIND_QUIETLY)
        MESSAGE(STATUS "Found LibNXCL: ${LIBNXCL_LIBRARIES}")
    ENDIF (NOT LIBNXCL_FIND_QUIETLY)
ELSE (LIBNXCL_NXSSHPROCESS_FOUND AND LIBNXCL_XID_FOUND)
    SET(LIBNXCL_FOUND FALSE)
    IF (LIBNXCL_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could NOT find acceptable version of LibNXCL (version 1.0).")
    ENDIF (LIBNXCL_FIND_REQUIRED)
ENDIF (LIBNXCL_NXSSHPROCESS_FOUND AND LIBNXCL_XID_FOUND)

MARK_AS_ADVANCED(LIBNXCL_INCLUDE_DIR LIBNXCL_LIBRARIES)

