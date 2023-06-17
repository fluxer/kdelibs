# for the kded module
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51005)

set(kpowermanager_LIB_SRCS
    kpowermanager.cpp
)

add_library(kpowermanager SHARED ${kpowermanager_LIB_SRCS})

target_link_libraries(kpowermanager PUBLIC
    ${QT_QTDBUS_LIBRARY}
    kdecore
)

set_target_properties(kpowermanager PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(kpowermanager)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kpowermanager_export.h
    kpowermanager.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    TARGETS kpowermanager
    EXPORT kdelibsLibraryTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

add_subdirectory(kded)


install(
    FILES
    kded/org.freedesktop.PowerManagement.xml
    kded/org.freedesktop.PowerManagement.Inhibit.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)

########### next target ###############

set(kpowermanager_helper_srcs
    kpowermanager_helper.cpp
)

add_executable(kpowermanager_helper ${kpowermanager_helper_srcs})
target_link_libraries(kpowermanager_helper PUBLIC
    kdecore
)

install(
    TARGETS kpowermanager_helper
    DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR}
)

kde4_install_auth_helper_files(kpowermanager_helper org.kde.kpowermanager.helper root)

