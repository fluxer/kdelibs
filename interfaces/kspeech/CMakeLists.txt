# dummy library for generate_export_header()
add_library(kspeech kspeech_dummy.cpp)

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
