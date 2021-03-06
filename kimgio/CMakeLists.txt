project(kimgio)

macro_optional_find_package(Jasper 2.0.20)
set_package_properties(Jasper PROPERTIES
    URL "http://www.ece.uvic.ca/~mdadams/jasper"
    PURPOSE "Support for JPEG-2000 images"
    TYPE OPTIONAL
)

macro_optional_find_package(OpenEXR)
set_package_properties(OpenEXR PROPERTIES
    URL "http://www.openexr.com"
    PURPOSE "Support for OpenEXR images"
    TYPE OPTIONAL
)

macro_optional_find_package(WebP)
set_package_properties(WebP PROPERTIES
    DESCRIPTION "A new image format for the Web"
    URL "https://developers.google.com/speed/webp"
    TYPE OPTIONAL
)

include_directories(${KDE4_KDEUI_INCLUDES})

##################################

if(JASPER_FOUND)
    include_directories(${JASPER_INCLUDE_DIR})
    kde4_add_plugin(kimg_jp2 jp2.cpp)
    target_link_libraries(kimg_jp2
        ${KDE4_KDECORE_LIBS}
        ${QT_QTGUI_LIBRARY}
        ${JASPER_LIBRARIES}
    )

    install(
        TARGETS kimg_jp2
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/kde4/plugins/imageformats
    )
    install(
        FILES
        jp2.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/qimageioplugins
    )
endif(JASPER_FOUND)

##################################

if(OPENEXR_FOUND)
    include_directories(${CMAKE_SOURCE_DIR}/kio ${OPENEXR_INCLUDE_DIR})
    add_definitions(${OPENEXR_DEFINITIONS})
    set_source_files_properties(exr.cpp PROPERTIES COMPILE_FLAGS "${KDE4_ENABLE_EXCEPTIONS}")
    set(kimg_exr_LIB_SRCS exr.cpp)
    kde4_add_plugin(kimg_exr ${kimg_exr_LIB_SRCS})
    target_link_libraries(kimg_exr
        ${KDE4_KDECORE_LIBS}
        ${QT_QTGUI_LIBRARY}
        ${OPENEXR_LIBRARIES}
    )

    install(
        TARGETS kimg_exr
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/kde4/plugins/imageformats
    )
    install(
        FILES exr.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/qimageioplugins
    )
endif(OPENEXR_FOUND)

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

    install(FILES webp.xml DESTINATION ${KDE4_XDG_MIME_INSTALL_DIR})
    update_xdg_mimetypes(${KDE4_XDG_MIME_INSTALL_DIR})
endif(WEBP_FOUND)

##################################

set(imgioplugins
    eps
    pcx
    ras
    rgb
    xcf
    dds
    psd
    xview
)

foreach(plugin ${imgioplugins})
    kde4_add_plugin(kimg_${plugin} ${plugin}.cpp)
    target_link_libraries(kimg_${plugin}
        ${KDE4_KDECORE_LIBS}
        ${QT_QTGUI_LIBRARY}
    )
    install(
        TARGETS kimg_${plugin}
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/kde4/plugins/imageformats
    )
endforeach()

##################################

set(kimg_pic_LIB_SRCS
    pic_read.cpp
    pic_write.cpp
    pic_io_plugin.cpp
    pic_io_handler.cpp
)

kde4_add_plugin(kimg_pic ${kimg_pic_LIB_SRCS})
target_link_libraries(kimg_pic
    ${KDE4_KDECORE_LIBS}
    ${QT_QTGUI_LIBRARY}
)

install(
    TARGETS kimg_pic
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/kde4/plugins/imageformats
)

########### install files ###############

install(
    FILES
    xv.desktop
    eps.desktop
    pcx.desktop
    ras.desktop
    rgb.desktop
    xcf.desktop
    dds.desktop
    psd.desktop
    hdr.desktop
    pic.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/qimageioplugins
)

install(
    FILES qimageio_plugin.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)


