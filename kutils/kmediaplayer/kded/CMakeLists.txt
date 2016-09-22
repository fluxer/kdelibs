########### next target ###############

set(kded_kaudioplayer_SRCS 
    kaudioplayer.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kaudioplayer.xml
)

qt4_generate_dbus_interface(kaudioplayer.h org.kde.kaudioplayer.xml )

kde4_add_plugin(kded_kaudioplayer ${kded_kaudioplayer_SRCS})
target_link_libraries(kded_kaudioplayer ${KDE4_KMEDIAPLAYER_LIBS})

install(
    TARGETS kded_kaudioplayer
    DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
    FILES kaudioplayer.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/kded
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kaudioplayer.xml
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR}
)


