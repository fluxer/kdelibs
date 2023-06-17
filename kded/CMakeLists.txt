project(kded)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

include_directories(${KDE4_KIO_INCLUDES})

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(KBUILDSYCOCA_NO_KCRASH TRUE)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config-kded.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kded.h
)

########### next target ###############

set(kded_SRCS
    kded.cpp
    kdedadaptor.cpp
    # just so that it gets generated
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kded.xml
)

add_executable(kded4 ${kded_SRCS})

target_link_libraries(kded4 ${KDE4_KIO_LIBS} ${X11_LIBRARIES})

install(
    TARGETS kded4
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)

qt4_generate_dbus_interface(kdedadaptor.h org.kde.kded.xml)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kded.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)

kde4_add_dbus_service(org.kde.kded.service.in)

########### next target ###############

set(kbuildsycoca_SRCS
    kbuildsycoca.cpp
    kbuildmimetypefactory.cpp
    kmimeassociations.cpp
    kbuildservicetypefactory.cpp
    kbuildservicefactory.cpp
    kbuildservicegroupfactory.cpp
    kbuildprotocolinfofactory.cpp
    kctimefactory.cpp
    vfolder_menu.cpp
)

if(KBUILDSYCOCA_NO_KCRASH)
    set(KBUILDSYCOCA_MAIN_LIBS ${KDE4_KDECORE_LIBS})
else()
    set(KBUILDSYCOCA_MAIN_LIBS ${KDE4_KDEUI_LIBS})
endif()

add_executable(kbuildsycoca4 ${kbuildsycoca_SRCS})

target_link_libraries(kbuildsycoca4
    ${KBUILDSYCOCA_MAIN_LIBS}
    ${QT_QTXML_LIBRARY}
)

install(
    TARGETS kbuildsycoca4
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)

########### install files ###############

install(
    FILES kde-applications.menu
    DESTINATION ${KDE4_SYSCONF_INSTALL_DIR}/xdg/menus
)

install(
    FILES
    kdedmodule.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)
