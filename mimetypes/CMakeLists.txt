install(
    FILES kde.xml
    DESTINATION ${KDE4_XDG_MIME_INSTALL_DIR}
)

update_xdg_mimetypes(${KDE4_XDG_MIME_INSTALL_DIR})
