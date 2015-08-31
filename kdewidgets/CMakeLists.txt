project(kdewidgets)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

add_subdirectory(pics)

include_directories(${KDE4_KIO_INCLUDES})


########### next target ###############

set(makekdewidgets_SRCS makekdewidgets.cpp )

add_executable(makekdewidgets ${makekdewidgets_SRCS})

target_link_libraries(makekdewidgets ${KDE4_KDECORE_LIBS})

# "export" this target too so we can use the LOCATION property of the imported target in
# FindKDE4Internal.cmake to get the full path to the installed executable instead of using FIND_PROGRAM(), Alex
install(TARGETS makekdewidgets  EXPORT kdelibsToolsTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

if(QT_QTDESIGNER_FOUND)
    KDE4_ADD_WIDGET_FILES(kde.widgets)

    set(kdewidgets_PART_SRCS
        classpreviews.cpp
        $<TARGET_OBJECTS:kde_autowidgets>
    )

    qt4_add_resources(kdewidgets_PART_SRCS kdewidgets.qrc)

    kde4_add_plugin(kdewidgets ${kdewidgets_PART_SRCS})

    target_link_libraries(kdewidgets ${KDE4_KIO_LIBS})

    install(TARGETS kdewidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )

    # kdewebkit widgets
    include_directories(${CMAKE_SOURCE_DIR}/kdewebkit)

    KDE4_ADD_WIDGET_FILES(kdewebkit.widgets)

    set(kdewebkitwidgets_PART_SRCS
        classpreviews.cpp
        $<TARGET_OBJECTS:kdewebkit_autowidgets>
    )

    qt4_add_resources(kdewebkitwidgets_PART_SRCS kdewebkitwidgets.qrc)

    kde4_add_plugin(kdewebkitwidgets ${kdewebkitwidgets_PART_SRCS})

    target_link_libraries(kdewebkitwidgets ${KDE4_KDEUI_LIBS} ${KDE4_KDEWEBKIT_LIBS} ${QT_QTWEBKIT_LIBRARY})

    install(TARGETS kdewebkitwidgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)
endif (QT_QTDESIGNER_FOUND)

########### install files ###############





