project(krichtexteditor)

find_package(KDELibs4 REQUIRED)

include_directories(${KDE4_INCLUDES})
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

set(krichtexteditorSources
    main.cpp
    krichtexteditor.cpp
)

add_executable(krichtexteditor ${krichtexteditorSources})
target_link_libraries(krichtexteditor ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS krichtexteditor DESTINATION ${KDE4_BIN_INSTALL_DIR})
install(FILES krichtexteditorui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/krichtexteditor)



