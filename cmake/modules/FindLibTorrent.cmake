# Try to find libtorrent-rasterbar, once done this will define:
#
#  LIBTORRENT_FOUND - system has libtorrent-rasterbar
#  LIBTORRENT_INCLUDES - the libtorrent-rasterbar include directory
#  LIBTORRENT_LIBRARIES - the libraries needed to use libtorrent-rasterbar
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBTORRENT QUIET libtorrent-rasterbar)

set(LIBTORRENT_INCLUDES ${PC_LIBTORRENT_INCLUDE_DIRS})
set(LIBTORRENT_LIBRARIES ${PC_LIBTORRENT_LIBRARIES})
set(LIBTORRENT_VERSION ${PC_LIBTORRENT_VERSION})

if(NOT LIBTORRENT_INCLUDES OR NOT LIBTORRENT_LIBRARIES)
    find_path(LIBTORRENT_INCLUDES
        NAMES torrent.hpp
        PATH_SUFFIXES libtorrent
        HINTS $ENV{LIBTORRENTDIR}/include
    )

    find_library(LIBTORRENT_LIBRARIES
        NAMES torrent-rasterbar
        HINTS $ENV{LIBTORRENTDIR}/lib
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibTorrent
    VERSION_VAR LIBTORRENT_VERSION
    REQUIRED_VARS LIBTORRENT_LIBRARIES LIBTORRENT_INCLUDES
)

mark_as_advanced(LIBTORRENT_INCLUDES LIBTORRENT_LIBRARIES)
