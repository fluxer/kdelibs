project(kparts)

add_definitions(
    -DKDE_DEFAULT_DEBUG_AREA=1000
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

include_directories(${KDE4_KIO_INCLUDES} ${kparts_BINARY_DIR})

########### next target ###############

set(kparts_LIB_SRCS
    part.cpp
    mainwindow.cpp
    event.cpp
    factory.cpp
)

add_library(kparts SHARED ${kparts_LIB_SRCS})

target_link_libraries(kparts PUBLIC
    kdecore
    kdeui
    kio
)

set_target_properties(kparts PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS kparts
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

########### install files ###############

install(
    FILES
    kpart.desktop
    krop.desktop
    krwp.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)

generate_export_header(kparts)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kparts_export.h
    part.h
    mainwindow.h
    event.h
    factory.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/kparts
)
