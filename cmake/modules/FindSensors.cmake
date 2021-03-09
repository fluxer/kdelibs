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

find_path(SENSORS_INCLUDE_DIR
    NAMES sensors/sensors.h
    HINTS $ENV{SENSORSDIR}/include
)

find_library(SENSORS_LIBRARIES
    NAMES sensors
    HINTS $ENV{SENSORSDIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sensors
    REQUIRED_VARS SENSORS_LIBRARIES SENSORS_INCLUDE_DIR
)

mark_as_advanced(SENSORS_INCLUDE_DIR SENSORS_LIBRARIES)
