project(kfile)

include_directories(
    ${KDE4_KIO_INCLUDES}
    ${CMAKE_SOURCE_DIR}/solid
    ${CMAKE_BINARY_DIR}/solid
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=250)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

set(kfile_LIB_SRCS
    kdiroperator.cpp
    kdiroperatordetailview.cpp
    # used in combination with kio/kio/kdirmodel.cpp
    kdirsortfilterproxymodel.cpp
    kfilebookmarkhandler.cpp
    kfilefiltercombo.cpp
    kfilewidget.cpp
    kfileplacesitem.cpp
    kfileplacesmodel.cpp
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
    kdefaultviewadapter.cpp
)

add_library(kfile SHARED ${kfile_LIB_SRCS})

target_link_libraries(kfile PUBLIC
    kdecore
    kdeui
    kio
    solid
)

set_target_properties(kfile PROPERTIES
    VERSION ${GENERIC_LIB_VERSION} 
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS kfile
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

generate_export_header(kfile)

install(
    FILES
    kdiroperator.h
    kdirsortfilterproxymodel.h
    kfilefiltercombo.h
    kfileplacesmodel.h
    kfileplacesview.h
    kfilepreviewgenerator.h
    kfilewidget.h
    kurlnavigator.h
    knewfilemenu.h
    knameandurlinputdialog.h
    ${CMAKE_CURRENT_BINARY_DIR}/kfile_export.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

############ module used by KFileDialog ##########

kde4_add_plugin(kfilemodule kfilemodule.cpp)

target_link_libraries(kfilemodule
    kfile
    kio
)

install(
    TARGETS kfilemodule
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES kfilemodule.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
