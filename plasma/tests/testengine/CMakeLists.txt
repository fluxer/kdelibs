project(plasma_testengine_dataengine)

find_package(KDELibs4 REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
)

kde4_add_plugin(plasma_engine_testengine testengine.cpp)

target_link_libraries(plasma_engine_testengine
    KDE4::kio
    KDE4::plasma
)

install(TARGETS plasma_engine_testengine DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-testengine.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
