project(kutils)

include_directories(
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KPARTS_INCLUDES}
    # keep it, required by kemoticons and others
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(kmediaplayer)

########### kemoticons ################

add_subdirectory(kemoticons)

set(kemoticons_LIB_SRCS
    kemoticons/kemoticons.cpp
    kemoticons/kemoticonstheme.cpp
    kemoticons/kemoticonsprovider.cpp
)

add_library(kemoticons ${LIBRARY_TYPE} ${kemoticons_LIB_SRCS})

target_link_libraries(kemoticons ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

set_target_properties(kemoticons PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    FILES
    kemoticons/kemoticonsTheme.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
)

generate_export_header(kemoticons)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kemoticons_export.h
    kemoticons/kemoticons.h
    kemoticons/kemoticonstheme.h
    kemoticons/kemoticonsprovider.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(
    TARGETS kemoticons
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

######## kidletime ####################

configure_file(
    kidletime/config-kidletime.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kidletime.h
)

set(kidletime_LIB_SRCS
    kidletime/kidletime.cpp
    kidletime/abstractsystempoller.cpp
    kidletime/widgetbasedpoller.cpp
)

if(Q_WS_X11)
    if (HAVE_XSCREENSAVER)
        set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS}
            kidletime/xscreensaverbasedpoller.cpp
        )
        qt4_add_dbus_interface(kidletime_LIB_SRCS
            kidletime/org.freedesktop.ScreenSaver.xml
            screensaver_interface
        )
    endif()

    if(HAVE_XSYNC)
        set(kidletime_LIB_SRCS
            ${kidletime_LIB_SRCS}
            kidletime/xsyncbasedpoller.cpp
        )
    endif()
endif(Q_WS_X11)

add_library(kidletime ${LIBRARY_TYPE} ${kidletime_LIB_SRCS})

target_link_libraries(kidletime
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTGUI_LIBRARY}
)

if (Q_WS_X11)
    if(HAVE_XSCREENSAVER)
        target_link_libraries(kidletime ${X11_Xscreensaver_LIB})
    endif (HAVE_XSCREENSAVER)

    if(HAVE_XSYNC)
        target_link_libraries(kidletime ${X11_Xext_LIB})
    endif()
    target_link_libraries(kidletime ${X11_LIBRARIES})
endif (Q_WS_X11)

set_target_properties(kidletime PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(kidletime)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kidletime_export.h
    kidletime/kidletime.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(
    TARGETS kidletime
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

########### kcmutils ###############

set(kcmutils_LIB_SRCS
    kcmoduleinfo.cpp
    kcmoduleloader.cpp
    kcmultidialog.cpp
    kcmoduleproxy.cpp
    kpluginselector.cpp
    kcmodulecontainer.cpp
    ksettingswidgetadaptor.cpp
    ksettings/dispatcher.cpp
    ksettings/dialog.cpp
    ksettings/pluginpage.cpp
    ksettings/componentsdialog.cpp
)

add_library(kcmutils ${LIBRARY_TYPE} ${kcmutils_LIB_SRCS})

target_link_libraries(kcmutils
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTGUI_LIBRARY}
)

set_target_properties(kcmutils PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(kcmutils)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kcmutils_export.h
    kcmoduleinfo.h
    kcmoduleloader.h
    kcmultidialog.h
    kcmoduleproxy.h
    kpluginselector.h
    kcmodulecontainer.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(
    FILES
    ksettings/dispatcher.h
    ksettings/dialog.h
    ksettings/pluginpage.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/ksettings
    COMPONENT Devel
)

install(
    TARGETS kcmutils
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

########### kprintutils ###############

add_library(kprintutils ${LIBRARY_TYPE} kprintpreview.cpp)

target_link_libraries(kprintutils
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
)

set_target_properties(kprintutils PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(kprintutils)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kprintutils_export.h
    kprintpreview.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(
    TARGETS kprintutils
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)
