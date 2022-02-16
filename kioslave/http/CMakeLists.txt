project(kioslave-http)

include_directories(${KDE4_KIO_INCLUDES})

########### next target ###############

kde4_add_plugin(kio_http http.cpp)

target_link_libraries(kio_http ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS kio_http DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### install files ###############

install(
    FILES
    http.protocol https.protocol
    webdav.protocol webdavs.protocol
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
