add_executable(kconfig_compiler kconfig_compiler.cpp)

target_link_libraries(kconfig_compiler
    ${QT_QTCORE_LIBRARY}
    ${QT_QTXML_LIBRARY}
)

install(
    TARGETS kconfig_compiler
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)
