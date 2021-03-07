# Try to find NetworkManager, once done this will define:
#
#  NETWORKMANAGER_FOUND - system has NetworkManager
#  NETWORKMANAGER_INCLUDE_DIRS - the NetworkManager include directories
#  NETWORKMANAGER_LIBRARIES - the libraries needed to use NetworkManager
#  NETWORKMANAGER_CFLAGS - Compiler switches required for using NetworkManager
#  NETWORKMANAGER_VERSION - version number of NetworkManager

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2007, Will Stephenson, <wstephenson@kde.org>
# Copyright (c) 2015 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (NETWORKMANAGER_INCLUDE_DIRS AND NM-UTIL_INCLUDE_DIRS AND NM-GLIB_INCLUDE_DIRS)
    # in cache already
    SET(NetworkManager_FIND_QUIETLY TRUE)
ENDIF ()

PKG_SEARCH_MODULE(NETWORKMANAGER libnm)
IF(NETWORKMANAGER_FOUND)
    PKG_SEARCH_MODULE(GLIB2 glib-2.0)

    SET(NM-UTIL_FOUND TRUE)
    SET(NM-GLIB_FOUND TRUE)
    SET(NETWORKMANAGER_INCLUDE_DIRS ${NETWORKMANAGER_INCLUDE_DIRS} ${GLIB2_INCLUDE_DIRS})
    SET(NM-UTIL_INCLUDE_DIRS ${NETWORKMANAGER_INCLUDE_DIRS})
    SET(NM-GLIB_INCLUDE_DIRS ${NETWORKMANAGER_INCLUDE_DIRS})

    IF (NOT NetworkManager_FIND_QUIETLY)
        MESSAGE(STATUS "Found NetworkManager ${NETWORKMANAGER_VERSION}: ${NETWORKMANAGER_INCLUDE_DIRS}")
    ENDIF()
ELSE()
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    PKG_SEARCH_MODULE( NETWORKMANAGER NetworkManager )
    PKG_SEARCH_MODULE( NM-UTIL libnm-util )
    PKG_SEARCH_MODULE( NM-GLIB libnm-glib )

    IF(NETWORKMANAGER_FOUND AND NM-UTIL_FOUND AND NM-GLIB_FOUND)
        IF (NOT NetworkManager_FIND_QUIETLY)
            MESSAGE(STATUS "Found NetworkManager ${NETWORKMANAGER_VERSION}: ${NETWORKMANAGER_LIBRARY_DIRS}")
            MESSAGE(STATUS "Found libnm-util: ${NM-UTIL_LIBRARY_DIRS}")
            MESSAGE(STATUS "Found libnm-glib: ${NM-GLIB_LIBRARY_DIRS}")
        ENDIF()
    ELSE()
        IF (NetworkManager_FIND_REQUIRED)
            MESSAGE(FATAL_ERROR "Could NOT find NetworkManager, libnm-util or libnm-glib, check FindPkgConfig output above!")
        ENDIF (NetworkManager_FIND_REQUIRED)
    ENDIF()
ENDIF()

MARK_AS_ADVANCED(NETWORKMANAGER_INCLUDE_DIRS NM-UTIL_INCLUDE_DIRS NM-GLIB_INCLUDE_DIRS)

