set(kfilepreview_image_SRCS kfilepreview_image.cpp)

kde4_add_plugin(kfilepreview_image ${kfilepreview_image_SRCS})
target_link_libraries(kfilepreview_image
    kio
    kexiv2
)

install(
    TARGETS kfilepreview_image
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES kfilepreview_image.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
