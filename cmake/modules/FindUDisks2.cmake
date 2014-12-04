# - Try to find UDisks2
# Once done, this will define
#
#  UDISKS2_FOUND - system has UDisks2
#  UDISKS2_INCLUDE_DIRS - the UDisks2 include directories
#  UDISKS2_LIBRARIES - link these to use UDisks2
# 
# Copyright (c) 2014, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


find_path(UDISKS2_INCLUDE_DIR udisks2/udisks/udisks.h)

find_library(UDISKS2_LIBRARIES NAMES udisks2 )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UDISKS2 DEFAULT_MSG UDISKS2_INCLUDE_DIR UDISKS2_LIBRARIES )

mark_as_advanced(UDISKS2_INCLUDE_DIR UDISKS2_LIBRARIES)