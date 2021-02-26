include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

########### next target ###############

kde4_add_plugin(emoticonstheme_pidgin pidgin_emoticons.cpp)

target_link_libraries(emoticonstheme_pidgin
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KEMOTICONS_LIBS}
)

########### install files ###############

install(
    TARGETS emoticonstheme_pidgin
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES emoticonstheme_pidgin.desktop
    DESTINATION  ${KDE4_SERVICES_INSTALL_DIR}
)
