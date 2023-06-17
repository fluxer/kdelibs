
# for strange kio/foo.h includes
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/kio)

########### next target ###############

add_executable(kmailservice kmailservice.cpp)

target_link_libraries(kmailservice ${KDE4_KDECORE_LIBS})

install(
    TARGETS kmailservice
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
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
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)
install(
    PROGRAMS ktelnetservice.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)
