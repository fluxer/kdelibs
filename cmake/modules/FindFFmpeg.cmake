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

find_path(LIBAVFORMAT_INCLUDES
    NAMES libavformat/avformat.h
    HINTS $ENV{FFMPEGDIR}/include
)

find_library(LIBAVFORMAT_LIBRARY
    NAMES avformat
    HINTS $ENV{FFMPEGDIR}/lib
)

find_path(LIBAVUTIL_INCLUDES
    NAMES libavutil/dict.h
    HINTS $ENV{FFMPEGDIR}/include
)

find_library(LIBAVUTIL_LIBRARY
    NAMES avformat
    HINTS $ENV{FFMPEGDIR}/lib
)

if (LIBAVFORMAT_INCLUDES AND LIBAVUTIL_INCLUDES)
    set(FFMPEG_INCLUDES ${LIBAVFORMAT_INCLUDES} ${LIBAVUTIL_INCLUDES})
endif()

if (LIBAVFORMAT_LIBRARY AND LIBAVUTIL_LIBRARY)
    set(FFMPEG_LIBRARIES ${LIBAVFORMAT_LIBRARY} ${LIBAVUTIL_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFmpeg
    REQUIRED_VARS FFMPEG_LIBRARIES FFMPEG_INCLUDES
)

mark_as_advanced(FFMPEG_INCLUDES FFMPEG_LIBRARIES)
