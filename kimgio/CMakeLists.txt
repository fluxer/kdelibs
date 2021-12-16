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
    )

    install(
        TARGETS kimg_magick
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/kde4/plugins/imageformats
    )
    install(
        FILES
        pbm.desktop
        pgm.desktop
        ppm.desktop
        xbm.desktop
        bmp.desktop
        gif.desktop
        ico.desktop
        jpg.desktop
        tga.desktop
        tiff.desktop
        dds.desktop
        eps.desktop
        exr.desktop
        jp2.desktop
        pcx.desktop
        psd.desktop
        xcf.desktop
        x-dcraw.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/qimageioplugins
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

    install(
        TARGETS kimg_webp
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/kde4/plugins/imageformats
    )
    install(
        FILES
        webp.desktop
        x-webp.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/qimageioplugins
    )
endif(WEBP_FOUND)

########### install files ###############

install(
    FILES qimageio_plugin.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)


