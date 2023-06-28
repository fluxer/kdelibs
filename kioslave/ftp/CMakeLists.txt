project(kioslave-ftp)

include_directories(${KDE4_KIO_INCLUDES})

########### next target ###############

kde4_add_plugin(kio_ftp ftp.cpp)

target_link_libraries(kio_ftp kdecore kio)

install(TARGETS kio_ftp DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES ftp.protocol DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
