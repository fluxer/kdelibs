kde4_add_plugin(kded_kssld kssld.cpp)

target_link_libraries(kded_kssld
    ${KDE4_KDECORE_LIBS}
    ${QT_QTNETWORK_LIBRARY}
)

install(
    TARGETS kded_kssld
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES kssld.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/kded
)




