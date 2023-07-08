########### next target ###############

set(kded_knetworkmanager_SRCS 
    kded_knetworkmanager.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.knetworkmanager.xml
)

qt4_generate_dbus_interface(kded_knetworkmanager.h org.kde.knetworkmanager.xml)

kde4_add_plugin(kded_knetworkmanager ${kded_knetworkmanager_SRCS})
target_link_libraries(kded_knetworkmanager PRIVATE
    ${QT_QTDBUS_LIBRARY}
    kdecore
    kdeui
    knetworkmanager
)

install(
    TARGETS kded_knetworkmanager
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES knetworkmanager.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/kded
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.knetworkmanager.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)

install(
    FILES knetworkmanager.notifyrc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/knetworkmanager
)
