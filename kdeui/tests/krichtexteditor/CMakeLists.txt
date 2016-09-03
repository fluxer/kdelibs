project(krichtexteditor)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

include_directories(${KDE4_INCLUDES})
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

set(krichtexteditorSources
    main.cpp
    krichtexteditor.cpp
)

add_executable(krichtexteditor ${krichtexteditorSources})
target_link_libraries(krichtexteditor ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS krichtexteditor DESTINATION ${BIN_INSTALL_DIR})
install(FILES krichtexteditorui.rc DESTINATION ${DATA_INSTALL_DIR}/krichtexteditor)



