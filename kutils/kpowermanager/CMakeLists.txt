# for the kded module
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51005)

set(kpowermanager_LIB_SRCS
    kpowermanager.cpp
)

add_library(kpowermanager ${LIBRARY_TYPE} ${kpowermanager_LIB_SRCS})

target_link_libraries(kpowermanager PUBLIC
    ${KDE4_KDECORE_LIBS}
    ${QT_QTDBUS_LIBRARY}
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
    COMPONENT Devel
)

install(
    TARGETS kpowermanager
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

add_subdirectory(kded)


install(
    FILES
    kded/org.freedesktop.PowerManagement.xml
    kded/org.freedesktop.PowerManagement.Inhibit.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)
