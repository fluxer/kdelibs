# dummy library for generate_export_header()
include_directories(${QT_INCLUDES})
add_library(kspeech kspeech.h)

generate_export_header(kspeech)

install(
    FILES
    dbustexttospeech.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kspeech_export.h
    kspeech.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
)

install(
    FILES
    org.kde.KSpeech.xml
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR}
)
