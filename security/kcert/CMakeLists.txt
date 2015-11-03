project(kcert)

include_directories(
    ${OPENSSL_INCLUDE_DIR}
    ${KDE4_KPARTS_INCLUDES}
    ${CMAKE_BINARY_DIR}/kio/kssl
    ${CMAKE_SOURCE_DIR}/kio/kssl
    ${KDE4_KIO_INCLUDES}
)

kde4_add_plugin(kcertpart kcertpart.cc)

target_link_libraries(kcertpart ${KDE4_KPARTS_LIBS})

install(
    TARGETS kcertpart
    DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
    FILES kcertpart.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)

install(
    FILES kcertpart.rc
    DESTINATION ${DATA_INSTALL_DIR}/kcertpart
)
