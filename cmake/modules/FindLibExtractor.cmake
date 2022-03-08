# Try to find GNU Libextractor, once done this will define:
#
#  LIBEXTRACTOR_FOUND - system has GNU Libextractor
#  LIBEXTRACTOR_INCLUDES - the GNU Libextractor include directory
#  LIBEXTRACTOR_LIBRARIES - the libraries needed to use GNU Libextractor
#
# Copyright (c) 2022 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBEXTRACTOR QUIET libextractor)

    set(LIBEXTRACTOR_INCLUDES ${PC_LIBEXTRACTOR_INCLUDE_DIRS})
    set(LIBEXTRACTOR_LIBRARIES ${PC_LIBEXTRACTOR_LIBRARIES})
endif()

set(LIBEXTRACTOR_VERSION ${PC_LIBEXTRACTOR_VERSION})

if(NOT LIBEXTRACTOR_INCLUDES OR NOT LIBEXTRACTOR_LIBRARIES)
    find_path(LIBEXTRACTOR_INCLUDES
        NAMES extractor.h
        HINTS $ENV{LIBEXTRACTORDIR}/include
    )

    find_library(LIBEXTRACTOR_LIBRARIES
        NAMES extractor
        HINTS $ENV{LIBEXTRACTORDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibExtractor
    VERSION_VAR LIBEXTRACTOR_VERSION
    REQUIRED_VARS LIBEXTRACTOR_LIBRARIES LIBEXTRACTOR_INCLUDES
)

mark_as_advanced(LIBEXTRACTOR_INCLUDES LIBEXTRACTOR_LIBRARIES)
