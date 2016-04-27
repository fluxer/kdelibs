add_subdirectory(icons)

include_directories(${KDE4_KIO_INCLUDES})

########### next target ###############

set(kimproxy_LIB_SRCS
    kimproxy.cpp
)

qt4_add_dbus_adaptor(kimproxy_LIB_SRCS org.kde.KIMIface.xml kimproxy.h KIMProxy)

qt4_add_dbus_interface(kimproxy_LIB_SRCS org.kde.KIMIface.xml kimiface )

add_library(kimproxy ${LIBRARY_TYPE} ${kimproxy_LIB_SRCS})

target_link_libraries(kimproxy PRIVATE ${KDE4_KIO_LIBS})
target_link_libraries(kimproxy PUBLIC ${QT_QTCORE_LIBRARY})

set_target_properties(
    kimproxy PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

########### install files ###############

generate_export_header(kimproxy)

install(
    TARGETS kimproxy
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kimproxy_export.h
    kimproxy.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(
    FILES
    dbusinstantmessenger.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
)
install(
    FILES
    kcm_instantmessenger.desktop
    DESTINATION ${DATA_INSTALL_DIR}/kcm_componentchooser
)

install(
    FILES
    org.kde.KIMIface.xml
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR}
)
