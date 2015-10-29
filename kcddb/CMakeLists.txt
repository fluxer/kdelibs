project(libkcddb)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QT_INCLUDES}
    ${KDE4_INCLUDES}
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
)

macro_optional_find_package(MusicBrainz5)
set_package_properties(MusicBrainz5 PROPERTIES
    DESCRIPTION "A library that provides access to metadata lookup on the MusicBrainz server"
    URL "http://www.musicbrainz.org"
    PURPOSE "Music metadata lookup for KDE multimedia applications through libkcddb. You need version 5.x of libmusicbrainz"
)

if(MUSICBRAINZ5_FOUND)
    set(HAVE_MUSICBRAINZ5 1)
    include_directories(${MUSICBRAINZ5_INCLUDE_DIRS})

    set(libmusicbrainz_SRCS ${libmusicbrainz_SRCS}
       musicbrainz/musicbrainzlookup.cpp
       musicbrainz/asyncmusicbrainzlookup.cpp)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
else(MUSICBRAINZ5_FOUND)
    set(HAVE_MUSICBRAINZ5 0)
    set(libmusicbrainz_SRCS)
endif(MUSICBRAINZ5_FOUND)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config-musicbrainz.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-musicbrainz.h
)

if(ENABLE_TESTING)
    add_subdirectory( test )
endif()
add_subdirectory( kcmcddb )
add_subdirectory( libkcddb )
