project(kparts)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

include_directories(${KDE4_KIO_INCLUDES} ${kparts_BINARY_DIR})

########### next target ###############

set(kparts_LIB_SRCS
    part.cpp
    plugin.cpp
    partmanager.cpp
    mainwindow.cpp
    event.cpp
    browserextension.cpp
    factory.cpp
    historyprovider.cpp
    browserinterface.cpp
    browserrun.cpp
    browseropenorsavequestion.cpp
    statusbarextension.cpp
    textextension.cpp
    fileinfoextension.cpp
    listingextension.cpp
)

add_library(kparts ${LIBRARY_TYPE} ${kparts_LIB_SRCS})

target_link_libraries(kparts PUBLIC
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
)

set_target_properties(kparts PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS kparts
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

########### install files ###############

install(
    FILES
    kpart.desktop
    krop.desktop
    krwp.desktop
    browserview.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)

generate_export_header(kparts)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kparts_export.h
    part.h
    plugin.h
    partmanager.h
    mainwindow.h
    event.h
    browserextension.h
    factory.h
    historyprovider.h
    browserinterface.h
    browserrun.h
    statusbarextension.h
    browseropenorsavequestion.h
    textextension.h
    fileinfoextension.h
    listingextension.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/kparts
    COMPONENT Devel
)
