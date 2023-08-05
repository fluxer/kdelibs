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
add_subdirectory(knetworkmanager)
add_subdirectory(kidletime)

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
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)
