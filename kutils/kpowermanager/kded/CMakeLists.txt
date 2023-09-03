########### next target ###############

set(kded_kpowermanager_SRCS 
    kded_kpowermanager.cpp
    kpowermanagerimpl.cpp
    kpowermanagerinhibitimpl.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kpowermanager.xml
)

qt4_add_dbus_adaptor(kded_kpowermanager_SRCS
    org.freedesktop.PowerManagement.xml
    kpowermanagerimpl.h
    KPowerManagerImpl
)

qt4_add_dbus_adaptor(kded_kpowermanager_SRCS
    org.freedesktop.PowerManagement.Inhibit.xml
    kpowermanagerinhibitimpl.h
    KPowerManagerInhibitImpl
)

qt4_generate_dbus_interface(kded_kpowermanager.h org.kde.kpowermanager.xml)

kde4_add_plugin(kded_kpowermanager ${kded_kpowermanager_SRCS})
target_link_libraries(kded_kpowermanager PRIVATE
    ${QT_QTDBUS_LIBRARY}
    kdecore
    kpowermanager
)

install(
    TARGETS kded_kpowermanager
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES kpowermanager.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/kded
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kpowermanager.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)

install(
    FILES kpowermanager.notifyrc
    DESTINATION ${KDE4_CONFIG_INSTALL_DIR}/notifications
)

