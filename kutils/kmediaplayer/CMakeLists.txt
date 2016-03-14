include_directories(
    ${MPV_INCLUDES}
)

add_definitions(
    -DMAKE_KMEDIAPLAYER_LIB
    -DKDE_DEFAULT_DEBUG_AREA=940
)

set(kmediaplayer_LIB_SRCS
    kmediaplayer.cpp
    kmediawidget.cpp
)

add_library(kmediaplayer ${LIBRARY_TYPE} ${kmediaplayer_LIB_SRCS})

target_link_libraries(kmediaplayer
    ${MPV_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
)

set_target_properties(kmediaplayer PROPERTIES
    VERSION ${KDE_NON_GENERIC_LIB_VERSION}
    SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
)

install(
    FILES
    kmediaplayer_export.h
    kmediaplayer.h
    kmediawidget.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(
    TARGETS kmediaplayer
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)
