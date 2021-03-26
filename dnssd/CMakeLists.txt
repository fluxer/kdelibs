project(dnssd)

include_directories( ${KDE4_KDECORE_INCLUDES} )

set(kdnssd_LIB_SRCS
    servicebase.cpp
    servicemodel.cpp
    domainmodel.cpp
)
kde4_add_kcfg_files(kdnssd_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/settings.kcfgc)

if(AVAHI_FOUND)
    include_directories( ${AVAHI_INCLUDE_DIR} )
    set(kdnssd_LIB_SRCS ${kdnssd_LIB_SRCS}
        avahi-domainbrowser.cpp
        avahi-servicebrowser.cpp
        avahi-remoteservice.cpp
        avahi-publicservice.cpp
        avahi-servicetypebrowser.cpp
        avahi_server_interface.cpp
        avahi_serviceresolver_interface.cpp
        avahi_entrygroup_interface.cpp
    )
    qt4_add_dbus_interface(
        kdnssd_LIB_SRCS
        org.freedesktop.Avahi.DomainBrowser.xml
        avahi_domainbrowser_interface
    )
    qt4_add_dbus_interface(
        kdnssd_LIB_SRCS
        org.freedesktop.Avahi.ServiceBrowser.xml
        avahi_servicebrowser_interface
    )
    qt4_add_dbus_interface(
        kdnssd_LIB_SRCS
        org.freedesktop.Avahi.ServiceTypeBrowser.xml
        avahi_servicetypebrowser_interface
    )
elseif(DNSSD_FOUND)
    include_directories( ${DNSSD_INCLUDE_DIR} )
    set(kdnssd_LIB_SRCS ${kdnssd_LIB_SRCS}
        mdnsd-domainbrowser.cpp
        mdnsd-remoteservice.cpp
        mdnsd-publicservice.cpp
        mdnsd-responder.cpp
        mdnsd-servicebrowser.cpp
        mdnsd-servicetypebrowser.cpp
    )
else()
    set(kdnssd_LIB_SRCS ${kdnssd_LIB_SRCS}
        dummy-domainbrowser.cpp
        dummy-remoteservice.cpp
        dummy-publicservice.cpp
        dummy-servicebrowser.cpp
        dummy-servicetypebrowser.cpp
    )
endif()

########### next target ###############

add_library(kdnssd ${LIBRARY_TYPE} ${kdnssd_LIB_SRCS})

target_link_libraries(kdnssd PRIVATE
    ${KDE4_KDECORE_LIBS}
    ${QT_QTNETWORK_LIBRARY}
)

if(DNSSD_FOUND)
    target_link_libraries(kdnssd PRIVATE
        ${DNSSD_LIBRARIES}
    )
endif()

target_link_libraries(kdnssd PUBLIC kdecore ${QT_QTCORE_LIBRARY} )

set_target_properties(kdnssd PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS kdnssd
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

########### install files ###############

generate_export_header(kdnssd)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kdnssd_export.h
    domainbrowser.h
    remoteservice.h
    servicetypebrowser.h
    publicservice.h 
    servicebase.h
    servicebrowser.h
    servicemodel.h
    domainmodel.h
    ${CMAKE_CURRENT_BINARY_DIR}/settings.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/dnssd
    COMPONENT Devel
)





