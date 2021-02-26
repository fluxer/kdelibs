include_directories(
    ${KDE4_INCLUDES}
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Weaver
    ${CMAKE_SOURCE_DIR}/WeaverGui
    ../../Weaver
    ../../WeaverGui
)

set(Jobs_SRCS
    Jobs.cpp
    WeaverObserverTest.cpp
)

add_executable(Jobs ${Jobs_SRCS})

target_link_libraries(Jobs
    ${KDE4_KDECORE_LIBS}
    ${KDE4_THREADWEAVER_LIBS}
    ${QT_QTGUI_LIBRARY}
    ThreadWeaverGui
)

install(TARGETS Jobs DESTINATION ${KDE4_BIN_INSTALL_DIR})
