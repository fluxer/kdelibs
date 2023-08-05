project(kidletime)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=7104)

set(kidletime_LIB_SRCS
    kidletime.cpp
    abstractsystempoller.cpp
    widgetbasedpoller.cpp
)

if (HAVE_XSCREENSAVER)
    set(kidletime_LIB_SRCS
        ${kidletime_LIB_SRCS}
        xscreensaverbasedpoller.cpp
    )
endif()

if(HAVE_XSYNC)
    set(kidletime_LIB_SRCS
        ${kidletime_LIB_SRCS}
        xsyncbasedpoller.cpp
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
target_link_libraries(kidletime PRIVATE
    ${X11_X11_LIB}
)

if(HAVE_XSCREENSAVER)
    target_link_libraries(kidletime PRIVATE ${X11_Xscreensaver_LIB})
endif()

if(HAVE_XSYNC)
    target_link_libraries(kidletime PRIVATE ${X11_Xext_LIB})
endif()

set_target_properties(kidletime PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(kidletime)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kidletime_export.h
    kidletime.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    TARGETS kidletime
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)
