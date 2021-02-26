include_directories(
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Weaver
    ../../Weaver
)

add_executable(Construction Construction.cpp)

target_link_libraries(Construction ${KDE4_THREADWEAVER_LIBS})

install(TARGETS Construction DESTINATION ${KDE4_BIN_INSTALL_DIR})
