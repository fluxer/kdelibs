########### next target ###############

set(kcmssl_SRCS
    kcmssl.cpp
    cacertificatespage.cpp
    displaycertdialog.cpp
)

kde4_add_plugin(kcm_ssl ${kcmssl_SRCS})
target_link_libraries(kcm_ssl ${KDE4_KIO_LIBS})

########### install files ###############

install(TARGETS kcm_ssl DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

install(
    FILES
    kcm_ssl.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
