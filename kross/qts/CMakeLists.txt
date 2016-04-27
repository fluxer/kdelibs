include_directories(
    ${CMAKE_SOURCE_DIR}
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kross
    ${CMAKE_BINARY_DIR}/kross
)

########### next target ###############

add_library(krossqtsplugin ${LIBRARY_TYPE} plugin.cpp)

target_link_libraries(krossqtsplugin
    ${QT_QTUITOOLS_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    krosscore
    ${QT_QTSCRIPT_LIBRARY}
)

set_target_properties(krossqtsplugin PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS krossqtsplugin DESTINATION ${PLUGIN_INSTALL_DIR}/kde4/plugins/script)

########### next target ###############

# add_executable(krossqtstest main.cpp)
# target_link_libraries(krossqtstest ${KDE4_KDEUI_LIBS} krossqtsplugin ${QT_QTSCRIPT_LIBRARY})
# set(krossqtsplugintest_SRCS main.cpp)
# target_link_libraries(krossqtsplugintest ${KDE4_KDECORE_LIBS} )
# add_executable(krossqtsplugintest ${krossqtsplugintest_SRCS})
# #install(TARGETS krossqtsplugintest DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

set(krossqts_PART_SRCS interpreter.cpp script.cpp)

kde4_add_plugin(krossqts ${krossqts_PART_SRCS})

target_link_libraries(krossqts
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDECORE_LIBS}
    krosscore
    ${QT_QTSCRIPT_LIBRARY}
)

install(TARGETS krossqts DESTINATION ${PLUGIN_INSTALL_DIR})
