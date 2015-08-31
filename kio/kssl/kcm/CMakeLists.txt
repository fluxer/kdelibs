########### next target ###############

set(kcmssl_SRCS
    kcmssl.cpp
    cacertificatespage.cpp
    displaycertdialog.cpp
    cacertificates.ui
    displaycert.ui
)

kde4_add_plugin(kcm_ssl ${kcmssl_SRCS})
target_link_libraries(kcm_ssl ${KDE4_KIO_LIBS})
install(TARGETS kcm_ssl DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############
install(FILES kcm_ssl.desktop DESTINATION ${SERVICES_INSTALL_DIR})
