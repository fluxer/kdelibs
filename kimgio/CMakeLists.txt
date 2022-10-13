project(kimgio)

include_directories(${KDE4_KDEUI_INCLUDES})

##################################

kde4_add_plugin(kimg_ico ico.cpp)
target_link_libraries(kimg_ico
    ${KDE4_KDECORE_LIBS}
    ${QT_QTGUI_LIBRARY}
)
set_target_properties(kimg_ico PROPERTIES
    OUTPUT_NAME ico
)

install(
    TARGETS kimg_ico
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats
)

##################################

if(WEBP_FOUND)
    include_directories(${WEBP_INCLUDES})
    kde4_add_plugin(kimg_webp webp.cpp)
    target_link_libraries(kimg_webp
        ${KDE4_KDECORE_LIBS}
        ${QT_QTGUI_LIBRARY}
        ${WEBP_LIBRARIES}
    )
    set_target_properties(kimg_webp PROPERTIES
        OUTPUT_NAME webp
    )

    install(
        TARGETS kimg_webp
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats
    )
endif(WEBP_FOUND)

##################################

if(LIBRAW_FOUND)
    include_directories(${LIBRAW_INCLUDE_DIR})
    kde4_add_plugin(kimg_raw raw.cpp)
    target_link_libraries(kimg_raw
        ${KDE4_KDECORE_LIBS}
        ${QT_QTGUI_LIBRARY}
        ${LIBRAW_LIBRARIES}
    )
    set_target_properties(kimg_raw PROPERTIES
        COMPILE_FLAGS "${KDE4_ENABLE_EXCEPTIONS}"
        OUTPUT_NAME raw
    )

    install(
        TARGETS kimg_raw
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats
    )
endif(LIBRAW_FOUND)

##################################

if(LIBJPEG_FOUND)
    include_directories(${LIBJPEG_INCLUDE_DIR})
    kde4_add_plugin(kimg_jpg jpg.cpp)
    target_link_libraries(kimg_jpg
        ${KDE4_KDECORE_LIBS}
        ${QT_QTGUI_LIBRARY}
        ${LIBJPEG_LIBRARIES}
    )
    set_target_properties(kimg_jpg PROPERTIES
        OUTPUT_NAME jpg
    )

    install(
        TARGETS kimg_jpg
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats
    )
endif(LIBJPEG_FOUND)

##################################

if(OPENJPEG_FOUND)
    include_directories(${OPENJPEG_INCLUDE_DIR})
    kde4_add_plugin(kimg_jp2 jp2.cpp)
    target_link_libraries(kimg_jp2
        ${KDE4_KDECORE_LIBS}
        ${QT_QTGUI_LIBRARY}
        ${OPENJPEG_LIBRARIES}
    )
    set_target_properties(kimg_jp2 PROPERTIES
        OUTPUT_NAME jp2
    )

    install(
        TARGETS kimg_jp2
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats
    )
endif(OPENJPEG_FOUND)
