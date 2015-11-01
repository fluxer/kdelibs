project(libkcompactdisc)

include_directories(
    ${QT_INCLUDES}
    ${KDE4_INCLUDES}
    ${KDE4_KDECORE_INCLUDES}
    ${CMAKE_BINARY_DIR}/solid
    ${CMAKE_SOURCE_DIR}/solid
)

find_package(Alsa)
alsa_configure_file(${CMAKE_CURRENT_BINARY_DIR}/config-alsa.h)

# TODO: drop wmlib entirely?
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(USE_WMLIB FALSE)
else ()
    set(USE_WMLIB TRUE)
endif()

set(kcompactdisc_LIB_SRCS
    kcompactdisc.cpp
    kcompactdisc_p.cpp
    phonon_interface.cpp
)

if(USE_WMLIB)
    set(kcompactdisc_LIB_SRCS
        ${kcompactdisc_LIB_SRCS}
        wmlib_interface.cpp
        wmlib/audio/audio.c
        wmlib/audio/audio_arts.c
        wmlib/audio/audio_alsa.c
        wmlib/audio/audio_sun.c
        wmlib/cdda.c
        wmlib/cddb.c
        wmlib/cdrom.c
        wmlib/wm_helpers.c
        wmlib/cdtext.c
        wmlib/scsi.c
        wmlib/plat_aix.c
        wmlib/plat_bsd386.c
        wmlib/plat_freebsd.c
        wmlib/plat_hpux.c
        wmlib/plat_irix.c
        wmlib/plat_linux.c
        wmlib/plat_svr4.c
        wmlib/plat_ultrix.c
        wmlib/plat_news.c
        wmlib/plat_openbsd.c
        wmlib/plat_osf1.c
        wmlib/plat_sun.c
        wmlib/plat_scor5.c
        wmlib/drv_sony.c
        wmlib/drv_toshiba.c
    )
    add_definitions(-DUSE_WMLIB=1)
endif(USE_WMLIB)

add_library(kcompactdisc SHARED ${kcompactdisc_LIB_SRCS})

target_link_libraries(kcompactdisc
    ${KDE4_KDECORE_LIBS}
    ${KDE4_PHONON_LIBS}
    ${KDE4_SOLID_LIBS}
)

if(HAVE_LIBASOUND2)
    target_link_libraries(kcompactdisc ${ASOUND_LIBRARY})
endif(HAVE_LIBASOUND2)
if (USE_WMLIB)
    target_link_libraries(kcompactdisc ${CMAKE_THREAD_LIBS_INIT})
endif(USE_WMLIB)

set_target_properties(kcompactdisc PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

########### install files ###############

install(
    FILES kcompactdisc.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/libkcompactdisc
    COMPONENT Devel
)

install(
    TARGETS kcompactdisc
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)
