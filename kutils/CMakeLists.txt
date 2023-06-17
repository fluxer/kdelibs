project(kutils)

include_directories(
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KPARTS_INCLUDES}
    # keep it, required by kidletime and others
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(kmediaplayer)
add_subdirectory(kexiv2)
add_subdirectory(kpasswdstore)
add_subdirectory(kpowermanager)
add_subdirectory(kdnssd)
add_subdirectory(karchive)
add_subdirectory(kemail)

######## kidletime ####################

set(kidletime_LIB_SRCS
    kidletime/kidletime.cpp
    kidletime/abstractsystempoller.cpp
    kidletime/widgetbasedpoller.cpp
)

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

set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/kutils/kidletime/xscreensaverbasedpoller.cpp
    ${CMAKE_SOURCE_DIR}/kutils/kidletime/xsyncbasedpoller.cpp
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE
)

add_library(kidletime SHARED ${kidletime_LIB_SRCS})

target_link_libraries(kidletime PUBLIC
    ${QT_QTGUI_LIBRARY}
    kdecore
    kdeui
)

if(HAVE_XSCREENSAVER)
    target_link_libraries(kidletime PRIVATE ${X11_Xscreensaver_LIB})
endif()

if(HAVE_XSYNC)
    target_link_libraries(kidletime PRIVATE ${X11_Xext_LIB})
endif()
target_link_libraries(kidletime PRIVATE ${X11_X11_LIB})

set_target_properties(kidletime PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(kidletime)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kidletime_export.h
    kidletime/kidletime.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    TARGETS kidletime
    EXPORT kdelibsLibraryTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
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

add_library(kcmutils SHARED ${kcmutils_LIB_SRCS})

target_link_libraries(kcmutils PUBLIC
    ${QT_QTGUI_LIBRARY}
    kdecore
    kdeui
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
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    FILES
    ksettings/dispatcher.h
    ksettings/dialog.h
    ksettings/pluginpage.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/ksettings
)

install(
    TARGETS kcmutils
    EXPORT kdelibsLibraryTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)
