include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw
)


########### fakehardwaretest ###############

kde4_add_test(solid-fakehardwaretest fakehardwaretest.cpp)

target_link_libraries(solid-fakehardwaretest
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    solid
)

add_definitions(-DTEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw/fakecomputer.xml")

########### solidhwtest ###############

kde4_add_test(solid-solidhwtest solidhwtest.cpp)

add_definitions(-DFAKE_COMPUTER_XML="${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw/fakecomputer.xml")

target_link_libraries(solid-solidhwtest
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    solid
)
