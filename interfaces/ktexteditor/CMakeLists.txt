include_directories(
    ${KDE4_KPARTS_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${CMAKE_SOURCE_DIR}/kutils/kemail
    ${CMAKE_BINARY_DIR}/kutils/kemail
)

########### next target ###############

set(ktexteditor_LIB_SRCS
    ktexteditor.cpp
    document.cpp
    range.cpp
    cursor.cpp
    templateinterface.cpp
    editorchooser.cpp
    attribute.cpp
    cursorfeedback.cpp
    rangefeedback.cpp
    smartcursor.cpp
    smartrange.cpp
    codecompletioninterface.cpp
    codecompletionmodel.cpp
    codecompletionmodelcontrollerinterface.cpp
    configinterface.cpp
    smartinterface.cpp
    highlightinterface.cpp
    movinginterface.cpp
    movingcursor.cpp
    movingrange.cpp
    movingrangefeedback.cpp
    messageinterface.cpp
)

add_library(ktexteditor SHARED ${ktexteditor_LIB_SRCS})

target_link_libraries(ktexteditor
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KEMAIL_LIBS}
    ${QT_QTDBUS_LIBRARY}
)

set_target_properties(ktexteditor PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(
    TARGETS ktexteditor
    EXPORT kdelibsLibraryTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

########### install files ###############

generate_export_header(ktexteditor)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ktexteditor_export.h
    factory.h
    editor.h
    document.h
    view.h
    plugin.h
    cursor.h
    range.h
    configpage.h
    searchinterface.h
    markinterface.h
    sessionconfiginterface.h
    editorchooser.h
    texthintinterface.h
    variableinterface.h
    templateinterface.h
    modificationinterface.h
    commandinterface.h
    smartinterface.h
    attribute.h
    cursorfeedback.h
    rangefeedback.h
    smartcursor.h
    smartcursornotifier.h
    smartcursorwatcher.h
    smartrange.h
    smartrangenotifier.h
    smartrangewatcher.h
    codecompletioninterface.h
    codecompletionmodel.h
    codecompletionmodelcontrollerinterface.h
    configinterface.h
    containerinterface.h
    annotationinterface.h
    loadsavefiltercheckplugin.h
    highlightinterface.h
    modeinterface.h
    movinginterface.h
    movingcursor.h
    movingrange.h
    movingrangefeedback.h
    recoveryinterface.h
    messageinterface.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/ktexteditor
)

install(
    FILES
    kcm_ktexteditor.desktop
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/kcm_componentchooser
)

install(
    FILES
    ktexteditor.desktop
    ktexteditorplugin.desktop
    ktexteditor_loadsavefiltercheckplugin.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)
