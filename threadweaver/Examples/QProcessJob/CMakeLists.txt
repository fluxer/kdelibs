include_directories(
    ${KDE4_INCLUDES}
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Weaver
    ${CMAKE_SOURCE_DIR}/Action
    ../../Weaver
)

set(QProcessJob_SRCS
    QProcessJob.cpp
    MainWindow.cpp
    main.cpp
)

add_executable(QProcessJob ${QProcessJob_SRCS})

target_link_libraries(QProcessJob
    ${KDE4_KDECORE_LIBS}
    ${KDE4_THREADWEAVER_LIBS}
    ${QT_QTGUI_LIBRARY}
)

install(TARGETS QProcessJob DESTINATION ${BIN_INSTALL_DIR})
