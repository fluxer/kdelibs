project(kdewidgets)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

add_subdirectory(pics)

include_directories(
    ${KDE4_KIO_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kutils/kmediaplayer
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

    set(kdewidgets_SRCS
        ${kdewidgets_SRCS}
        classpreviews.cpp
    )

    qt4_add_resources(kdewidgets_SRCS kdewidgets.qrc)

    kde4_add_plugin(kdewidgets ${kdewidgets_SRCS})

    target_link_libraries(kdewidgets ${KDE4_KIO_LIBS} kmediaplayer)

    install(
        TARGETS kdewidgets
        DESTINATION ${PLUGIN_INSTALL_DIR}/kde4/plugins/designer
    )

    if(QT_QTWEBKIT_FOUND)
        include_directories(
            ${CMAKE_SOURCE_DIR}/kdewebkit
            ${CMAKE_BINARY_DIR}/kdewebkit
        )

        kde4_add_widget(kdewebkitwidgets_SRCS kdewebkit.widgets)

        set(kdewebkitwidgets_SRCS
            ${kdewebkitwidgets_SRCS}
            classpreviews.cpp
        )

        qt4_add_resources(kdewebkitwidgets_SRCS kdewebkitwidgets.qrc)

        kde4_add_plugin(kdewebkitwidgets ${kdewebkitwidgets_SRCS})

        target_link_libraries(kdewebkitwidgets
            ${KDE4_KDEUI_LIBS}
            ${KDE4_KDEWEBKIT_LIBS}
            ${QT_QTWEBKIT_LIBRARY}
        )

        install(
            TARGETS kdewebkitwidgets
            DESTINATION ${PLUGIN_INSTALL_DIR}/kde4/plugins/designer
        )
    endif()
endif(QT_QTDESIGNER_FOUND)

