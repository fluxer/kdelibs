# Try to find FFmpegthumbnailer library, once done this will define:
#
#  FFMPEGTHUMBNAILER_FOUND - system has FFmpegthumbnailer
#  FFMPEGTHUMBNAILER_INCLUDES - the FFmpegthumbnailer include directory
#  FFMPEGTHUMBNAILER_LIBRARIES - the libraries needed to use FFmpegthumbnailer
#
# Copyright (c) 2020 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_FFMPEGTHUMBNAILER QUIET libffmpegthumbnailer)

set(FFMPEGTHUMBNAILER_INCLUDES ${PC_FFMPEGTHUMBNAILER_INCLUDE_DIRS})
set(FFMPEGTHUMBNAILER_LIBRARIES ${PC_FFMPEGTHUMBNAILER_LIBRARIES})
set(FFMPEGTHUMBNAILER_VERSION ${PC_FFMPEGTHUMBNAILER_VERSION})

if(NOT FFMPEGTHUMBNAILER_INCLUDES OR NOT FFMPEGTHUMBNAILER_LIBRARIES)
    find_path(FFMPEGTHUMBNAILER_INCLUDES
        NAMES libffmpegthumbnailer/videothumbnailer.h
        HINTS $ENV{FFMPEGTHUMBNAILER}/include
    )

    find_library(FFMPEGTHUMBNAILER_LIBRARIES
        NAMES ffmpegthumbnailer
        HINTS $ENV{FFMPEGTHUMBNAILER}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFmpegThumbnailer
    VERSION_VAR FFMPEGTHUMBNAILER_VERSION
    REQUIRED_VARS FFMPEGTHUMBNAILER_LIBRARIES FFMPEGTHUMBNAILER_INCLUDES
)

mark_as_advanced(FFMPEGTHUMBNAILER_INCLUDES FFMPEGTHUMBNAILER_LIBRARIES)
