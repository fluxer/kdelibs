include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

########### next target ###############

kde4_add_plugin(emoticonstheme_adium adium_emoticons.cpp)

target_link_libraries(emoticonstheme_adium
    kemoticons
    ${KDE4_KDEUI_LIBS}
    ${QT_QTXML_LIBRARY}
)

########### install files ###############

install(
    TARGETS emoticonstheme_adium
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES emoticonstheme_adium.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
