if(MPV_FOUND)
    include_directories(${MPV_INCLUDES})
endif()

# for the kded module
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=940)

set(kmediaplayer_LIB_SRCS
    kmediaplayer.cpp
    kmediawidget.cpp
)

add_library(kmediaplayer SHARED ${kmediaplayer_LIB_SRCS})

target_link_libraries(kmediaplayer PUBLIC
    kdecore
    kdeui
)

if(MPV_FOUND)
    target_link_libraries(kmediaplayer PRIVATE ${MPV_LIBRARIES})
endif()

set_target_properties(kmediaplayer PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(kmediaplayer)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kmediaplayer_export.h
    kmediaplayer.h
    kmediawidget.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    TARGETS kmediaplayer
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

add_subdirectory(kded)
