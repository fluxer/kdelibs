project(kdeclarative)

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(KDECLARATIVE_NO_KIO TRUE)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config-kdeclarative.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kdeclarative.h
)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_KIO_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(kdeclarative_LIB_SRCS
    kdeclarative.cpp
    private/engineaccess.cpp
    private/kiconprovider.cpp
    bindings/qscriptnonguibookkeeping.cpp
    bindings/i18n.cpp
    bindings/icon.cpp
    bindings/image.cpp
    bindings/url.cpp
)

add_library(kdeclarative SHARED ${kdeclarative_LIB_SRCS})

set_target_properties(kdeclarative PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

target_link_libraries(kdeclarative PUBLIC
    ${QT_QTSCRIPT_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    kdecore
    kdeui
)

generate_export_header(kdeclarative)

install(
    FILES
    kdeclarative.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdeclarative_export.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    TARGETS kdeclarative
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)
