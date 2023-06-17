project(kdewidgets)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

add_subdirectory(pics)

include_directories(
    ${KDE4_KIO_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kutils/kmediaplayer
    # for the export header
    ${CMAKE_BINARY_DIR}/kutils/kmediaplayer
)

########### next target ###############

add_executable(makekdewidgets makekdewidgets.cpp)
target_link_libraries(makekdewidgets kdecore)

install(
    TARGETS makekdewidgets
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)

########### next target ###############

if(QT_QTUITOOLS_FOUND)
    kde4_add_widget(kdewidgets_SRCS kde.widgets)

    kde4_add_plugin(kdewidgets ${kdewidgets_SRCS})

    target_link_libraries(kdewidgets kio kmediaplayer)

    install(
        TARGETS kdewidgets
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/plugins/designer
    )
endif(QT_QTUITOOLS_FOUND)

