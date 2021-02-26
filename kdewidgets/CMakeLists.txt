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

target_link_libraries(makekdewidgets ${KDE4_KDECORE_LIBS})

install(
    TARGETS makekdewidgets
    EXPORT kdelibsToolsTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

########### next target ###############

if(QT_QTDESIGNER_FOUND)
    kde4_add_widget(kdewidgets_SRCS kde.widgets)

    qt4_add_resources(kdewidgets_SRCS kdewidgets.qrc)

    kde4_add_plugin(kdewidgets ${kdewidgets_SRCS})

    target_link_libraries(kdewidgets ${KDE4_KIO_LIBS} ${KDE4_KMEDIAPLAYER_LIBS})

    install(
        TARGETS kdewidgets
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/kde4/plugins/designer
    )
endif(QT_QTDESIGNER_FOUND)

