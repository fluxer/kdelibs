# Try to find the FFmpeg library, once done this will define:
#
#  FFMPEG_FOUND - system has FFmpeg
#  FFMPEG_INCLUDES - the FFmpeg include directory
#  FFMPEG_LIBRARIES - the libraries needed to use FFmpeg
#
# Copyright (c) 2022 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBAVCODEC QUIET libavcodec)
    pkg_check_modules(PC_LIBAVFORMAT QUIET libavformat)
    pkg_check_modules(PC_LIBAVUTIL QUIET libavutil)

    set(LIBAVCODEC_INCLUDES ${PC_LIBAVCODEC_INCLUDE_DIRS})
    set(LIBAVCODEC_LIBRARIES ${PC_LIBAVCODEC_LIBRARIES})
    set(LIBAVFORMAT_INCLUDES ${PC_LIBAVFORMAT_INCLUDE_DIRS})
    set(LIBAVFORMAT_LIBRARIES ${PC_LIBAVFORMAT_LIBRARIES})
    set(LIBAVUTIL_INCLUDES ${PC_LIBAVUTIL_INCLUDE_DIRS})
    set(LIBAVUTIL_LIBRARIES ${PC_LIBAVUTIL_LIBRARIES})
endif()

if(NOT LIBAVCODEC_INCLUDES OR NOT LIBAVCODEC_LIBRARIES)
    find_path(LIBAVCODEC_INCLUDES
        NAMES libavcodec/codec_desc.h
        HINTS $ENV{FFMPEGDIR}/include
    )

    find_library(LIBAVCODEC_LIBRARIES
        NAMES avcodec
        HINTS $ENV{FFMPEGDIR}/lib
    )
endif()

if(NOT LIBAVFORMAT_INCLUDES OR NOT LIBAVFORMAT_LIBRARIES)
    find_path(LIBAVFORMAT_INCLUDES
        NAMES libavformat/avformat.h
        HINTS $ENV{FFMPEGDIR}/include
    )

    find_library(LIBAVFORMAT_LIBRARIES
        NAMES avformat
        HINTS $ENV{FFMPEGDIR}/lib
    )
endif()

if(NOT LIBAVUTIL_INCLUDES OR NOT LIBAVUTIL_LIBRARIES)
    find_path(LIBAVUTIL_INCLUDES
        NAMES libavutil/dict.h
        HINTS $ENV{FFMPEGDIR}/include
    )

    find_library(LIBAVUTIL_LIBRARIES
        NAMES avformat
        HINTS $ENV{FFMPEGDIR}/lib
    )
endif()

if (LIBAVCODEC_INCLUDES AND LIBAVFORMAT_INCLUDES AND LIBAVUTIL_INCLUDES)
    set(FFMPEG_INCLUDES ${LIBAVCODEC_INCLUDES} ${LIBAVFORMAT_INCLUDES} ${LIBAVUTIL_INCLUDES})
endif()

if (LIBAVCODEC_LIBRARIES AND LIBAVFORMAT_LIBRARIES AND LIBAVUTIL_LIBRARIES)
    set(FFMPEG_LIBRARIES ${LIBAVCODEC_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVUTIL_LIBRARIES})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFmpeg
    REQUIRED_VARS FFMPEG_LIBRARIES FFMPEG_INCLUDES
)

mark_as_advanced(FFMPEG_INCLUDES FFMPEG_LIBRARIES)
