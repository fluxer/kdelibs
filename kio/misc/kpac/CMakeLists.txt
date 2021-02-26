
if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(KPAC_NO_SOLID TRUE)
endif()

include_directories(${CMAKE_SOURCE_DIR}/kded)

if(NOT KPAC_NO_SOLID)
    include_directories(${CMAKE_BINARY_DIR}/solid)
    include_directories(${CMAKE_SOURCE_DIR}/solid)
    set(KPAC_EXTRA_LIBS ${KDE4_SOLID_LIBS})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

include(ConfigureChecks.cmake)
configure_file(
    config-kpac.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kpac.h
)

########### next target ###############

set(kded_proxyscout_PART_SRCS
    proxyscout.cpp
    script.cpp
    downloader.cpp
    discovery.cpp
)

kde4_add_plugin(kded_proxyscout ${kded_proxyscout_PART_SRCS})

target_link_libraries(kded_proxyscout
    ${QT_QTSCRIPT_LIBRARY}
    ${KPAC_EXTRA_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
)

# this needs -lresolv e.g on Slackware, but not on FreeBSD
if(HAVE_RESOLV_LIBRARY)
    target_link_libraries(kded_proxyscout resolv)
endif()

install(
    TARGETS kded_proxyscout
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

########### next target ###############

add_executable(kpac_dhcp_helper kpac_dhcp_helper.c)

if(HAVE_NSL_LIBRARY)
    # Assume Solaris
    target_link_libraries(kpac_dhcp_helper nsl)
endif()

if (HAVE_SOCKET_LIBRARY)
    target_link_libraries(kpac_dhcp_helper socket)
endif()

install(
    TARGETS kpac_dhcp_helper
    DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR}
)

########### install files ###############

install(
    FILES
    proxyscout.notifyrc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/proxyscout
)
install(
    FILES
    proxyscout.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/kded
)

