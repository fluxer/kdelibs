########### next target ###############

set(kded_kpasswdstore_SRCS 
    kded_kpasswdstore.cpp
    kpasswdstoreimpl.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kpasswdstore.xml
)

qt4_generate_dbus_interface(kded_kpasswdstore.h org.kde.kpasswdstore.xml)

kde4_add_plugin(kded_kpasswdstore ${kded_kpasswdstore_SRCS})
target_link_libraries(kded_kpasswdstore PRIVATE
    kdecore
    kdeui
)

if(OPENSSL_FOUND)
    target_link_libraries(kded_kpasswdstore PRIVATE ${OPENSSL_LIBRARIES})
endif()

install(
    TARGETS kded_kpasswdstore
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES kpasswdstore.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/kded
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kpasswdstore.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)


