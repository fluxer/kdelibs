project(plasma-containmentactions-test)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
)

kde4_add_plugin(plasma_containmentactions_test test.cpp)
target_link_libraries(plasma_containmentactions_test ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_containmentactions_test DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-test.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
