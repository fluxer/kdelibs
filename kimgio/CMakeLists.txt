project(kimgio)

include_directories(${KDE4_KDEUI_INCLUDES})

##################################

if(IMAGEMAGICK_FOUND)
    include_directories(${ImageMagick_INCLUDE_DIRS})
    kde4_add_plugin(kimg_magick magick.cpp)
    target_link_libraries(kimg_magick
        ${KDE4_KDECORE_LIBS}
        ${QT_QTGUI_LIBRARY}
        ${ImageMagick_LIBRARIES}
    )
    set_target_properties(kimg_magick PROPERTIES
        COMPILE_FLAGS "${KDE4_ENABLE_EXCEPTIONS}"
        OUTPUT_NAME magick
    )

    install(
        TARGETS kimg_magick
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/plugins/imageformats
    )
endif(IMAGEMAGICK_FOUND)

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
