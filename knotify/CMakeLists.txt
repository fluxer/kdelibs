project(knotify)

include_directories(
    ${KDE4_KIO_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/kutils/kspeech
    ${CMAKE_BINARY_DIR}/kutils/kspeech
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

########### next target ###############

set(knotifyconfig_LIB_SRCS
    knotifyconfigactionswidget.cpp
    knotifyconfigelement.cpp
    knotifyeventlist.cpp
    knotifyconfigwidget.cpp
)

add_library(knotifyconfig ${LIBRARY_TYPE} ${knotifyconfig_LIB_SRCS})

# Needs KIO for KUrlRequester
target_link_libraries(knotifyconfig PUBLIC
    ${KDE4_KIO_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
)

set_target_properties(knotifyconfig PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(
    TARGETS knotifyconfig
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

########### install files ###############

generate_export_header(knotifyconfig)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/knotifyconfig_export.h
    knotifyconfigwidget.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)
