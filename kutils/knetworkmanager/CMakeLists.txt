# for the kded module
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51011)

set(knetworkmanager_LIB_SRCS
    knetworkmanager.cpp
)

add_library(knetworkmanager SHARED ${knetworkmanager_LIB_SRCS})

target_link_libraries(knetworkmanager PUBLIC
    ${QT_QTDBUS_LIBRARY}
    kdecore
)

set_target_properties(knetworkmanager PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(knetworkmanager)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/knetworkmanager_export.h
    knetworkmanager.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    TARGETS knetworkmanager
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

add_subdirectory(kded)
