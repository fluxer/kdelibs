project(kdewidgets)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

add_subdirectory(pics)

include_directories(${KDE4_KIO_INCLUDES})

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
    kde4_add_widget(kdewidgets_PART_SRCS kde.widgets)

    set(kdewidgets_PART_SRCS
        classpreviews.cpp
    )

    qt4_add_resources(kdewidgets_PART_SRCS kdewidgets.qrc)

    kde4_add_plugin(kdewidgets ${kdewidgets_PART_SRCS})

    target_link_libraries(kdewidgets ${KDE4_KIO_LIBS})

    # kdewebkit widgets
    include_directories(${CMAKE_SOURCE_DIR}/kdewebkit)

    kde4_add_widget(kdewebkitwidgets_PART_SRCS kdewebkit.widgets)

    set(kdewebkitwidgets_PART_SRCS
        classpreviews.cpp
    )

    qt4_add_resources(kdewebkitwidgets_PART_SRCS kdewebkitwidgets.qrc)

    kde4_add_plugin(kdewebkitwidgets ${kdewebkitwidgets_PART_SRCS})

    target_link_libraries(kdewebkitwidgets
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KDEWEBKIT_LIBS}
        ${QT_QTWEBKIT_LIBRARY}
    )

    install(
        TARGETS kdewidgets kdewebkitwidgets
        DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer
    )
endif(QT_QTDESIGNER_FOUND)

