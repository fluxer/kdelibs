project(kfile)

include_directories(
    ${KDE4_KIO_INCLUDES}
    ${CMAKE_SOURCE_DIR}/solid
    ${CMAKE_BINARY_DIR}/solid
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

set(kfile_LIB_SRCS
    kdiroperator.cpp
    kdiroperatordetailview.cpp
    kdirselectdialog.cpp
    # used in combination with kio/kio/kdirmodel.cpp
    kdirsortfilterproxymodel.cpp
    kfilebookmarkhandler.cpp
    kfilefiltercombo.cpp
    kfiletreeview.cpp
    kfilewidget.cpp
    kfileplacesitem.cpp
    kfileplacesmodel.cpp
    kfileplacessharedbookmarks.cpp
    kfileplacesview.cpp
    kfileplaceeditdialog.cpp
    kfilepreviewgenerator.cpp
    knameandurlinputdialog.cpp
    knewfilemenu.cpp
    kurlnavigatordropdownbutton.cpp
    kurlnavigatorbuttonbase.cpp
    kurlnavigatorbutton.cpp
    kurlnavigatorplacesselector.cpp
    kurlnavigatorprotocolcombo.cpp
    kurlnavigatortogglebutton.cpp
    kurlnavigator.cpp
    kurlnavigatormenu.cpp
)

add_library(kfile ${LIBRARY_TYPE} ${kfile_LIB_SRCS})

target_link_libraries(kfile PUBLIC
    ${KDE4_SOLID_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDECORE_LIBS}
)

set_target_properties(kfile PROPERTIES
    VERSION ${GENERIC_LIB_VERSION} 
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS kfile
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

generate_export_header(kfile)

install(
    FILES
    kdiroperator.h
    kdirselectdialog.h
    kdirsortfilterproxymodel.h
    kfilefiltercombo.h
    kfileplacesmodel.h
    kfileplacesview.h
    kfilepreviewgenerator.h
    kfiletreeview.h
    kfilewidget.h
    kurlnavigator.h
    knewfilemenu.h
    knameandurlinputdialog.h
    ${CMAKE_CURRENT_BINARY_DIR}/kfile_export.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

############ module used by KFileDialog ##########

kde4_add_plugin(kfilemodule kfilemodule.cpp)

target_link_libraries(kfilemodule
    ${KDE4_KFILE_LIBS}
    ${KDE4_KIO_LIBS}
)

install(
    TARGETS kfilemodule
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES kfilemodule.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
