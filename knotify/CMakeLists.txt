project(knotify)

include_directories(
    ${KDE4_KIO_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kutils/kmediaplayer
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
target_link_libraries(knotifyconfig
    ${KDE4_KIO_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${QT_QTGUI_LIBRARY}
    kmediaplayer
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

install(
    FILES
    knotifyconfig_export.h
    knotifyconfigwidget.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)
