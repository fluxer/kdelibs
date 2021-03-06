include_directories(
    ${CMAKE_SOURCE_DIR}
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kfile
    # for the export headers
    ${CMAKE_BINARY_DIR}/kross
    ${CMAKE_BINARY_DIR}/kfile
)

if(QT_QTDESIGNER_FOUND)
    ############################################
    # the forms module

    kde4_add_plugin(krossmoduleforms form.cpp)

    target_link_libraries(krossmoduleforms
        ${QT_QTUITOOLS_LIBRARY}
        ${KDE4_KPARTS_LIBS}
        ${KDE4_KFILE_LIBS}
        krosscore
    )
    install(TARGETS krossmoduleforms DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
endif()

############################################
# the kdetranslation module

kde4_add_plugin(krossmodulekdetranslation translation.cpp)

target_link_libraries(krossmodulekdetranslation
    ${KDE4_KPARTS_LIBS}
    ${KDE4_CORE_LIBS}
    krosscore
)
install(TARGETS krossmodulekdetranslation DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
