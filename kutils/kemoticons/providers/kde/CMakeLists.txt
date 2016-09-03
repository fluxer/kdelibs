include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

########### next target ###############

kde4_add_plugin(emoticonstheme_kde kde_emoticons.cpp)

target_link_libraries(emoticonstheme_kde
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KEMOTICONS_LIBS}
    ${QT_QTXML_LIBRARY}
)

########### install files ###############

install(
    TARGETS emoticonstheme_kde
    DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
    FILES emoticonstheme_kde.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)
