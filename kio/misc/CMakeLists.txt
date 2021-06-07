
# for strange kio/foo.h includes
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/kio)

# arpa/nameser.h
add_subdirectory(kpac)
add_subdirectory(kntlm)

########### next target ###############

add_executable(kmailservice kmailservice.cpp)

target_link_libraries(kmailservice ${KDE4_KDECORE_LIBS})

install(
    TARGETS kmailservice
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)
install(
    PROGRAMS kmailservice.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

########### next target ###############

add_executable(ktelnetservice ktelnetservice.cpp)

target_link_libraries(ktelnetservice ${KDE4_KDEUI_LIBS})

install(
    TARGETS ktelnetservice
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)
install(
    PROGRAMS ktelnetservice.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

########### install files ###############

install(
    FILES
    rtsp.protocol
    mms.protocol
    mmst.protocol
    mmsu.protocol
    pnm.protocol
    rtspt.protocol
    rtspu.protocol
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
