# Try to find the sensors directory library, once done this will define:
#
#  SENSORS_FOUND - system has SENSORS
#  SENSORS_INCLUDE_DIR - the SENSORS include directory
#  SENSORS_LIBRARIES - The libraries needed to use SENSORS
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

FIND_PATH(SENSORS_INCLUDE_DIR
    NAMES sensors/sensors.h
    HINTS $ENV{SENSORSDIR}/include
)

FIND_LIBRARY(SENSORS_LIBRARIES
    NAMES sensors
    HINTS $ENV{SENSORSDIR}/lib
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Sensors
    REQUIRED_VARS SENSORS_LIBRARIES SENSORS_INCLUDE_DIR
)

MARK_AS_ADVANCED(SENSORS_INCLUDE_DIR SENSORS_LIBRARIES)

