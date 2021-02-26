
########### install files ###############
install(
    FILES
    byteseditinterface.h
    valuecolumninterface.h
    charcolumninterface.h
    zoominterface.h
    clipboardinterface.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/khexedit
)

install(
    FILES
    kbytesedit.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)
