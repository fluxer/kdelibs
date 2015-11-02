project(kioslave-metainfo)

include_directories(${KDE4_KIO_INCLUDES})

########### next target ###############

kde4_add_plugin(kio_metainfo metainfo.cpp)

target_link_libraries(kio_metainfo ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS kio_metainfo DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES metainfo.protocol DESTINATION ${SERVICES_INSTALL_DIR})
