########### next target ###############

set(kded_kaudioplayer_SRCS 
    kaudioplayer.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kaudioplayer.xml
)

qt4_generate_dbus_interface(kaudioplayer.h org.kde.kaudioplayer.xml )

kde4_add_plugin(kded_kaudioplayer ${kded_kaudioplayer_SRCS})
target_link_libraries(kded_kaudioplayer kmediaplayer)

install(
    TARGETS kded_kaudioplayer
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES kaudioplayer.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/kded
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kaudioplayer.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)


