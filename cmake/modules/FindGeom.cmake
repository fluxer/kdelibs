# Try to find geom, once done this will define:
#
#  GEOM_FOUND - system has geom
#  GEOM_INCLUDES - the geom include directory
#  GEOM_LIBRARIES - the libraries needed to use geom
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(GEOM_INCLUDES
    NAMES libgeom.h
    HINTS $ENV{GEOMDIR}/include
)

find_library(GEOM_LIBRARIES
    NAMES geom
    HINTS $ENV{GEOMDIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Geom
    REQUIRED_VARS GEOM_LIBRARIES GEOM_INCLUDES
)

mark_as_advanced(GEOM_INCLUDES GEOM_LIBRARIES)
