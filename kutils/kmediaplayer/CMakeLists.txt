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

add_library(kmediaplayer ${LIBRARY_TYPE} ${kmediaplayer_LIB_SRCS})

target_link_libraries(kmediaplayer PUBLIC
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
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
    COMPONENT Devel
)

install(
    TARGETS kmediaplayer
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

add_subdirectory(kded)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
