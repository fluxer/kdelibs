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
    ${CMAKE_SOURCE_DIR}/kio
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
    bindings/url.cpp
)

add_library(kdeclarative ${LIBRARY_TYPE} ${kdeclarative_LIB_SRCS})

set_target_properties(kdeclarative PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

target_link_libraries(kdeclarative
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTSCRIPT_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
)

if(KATIE_FOUND)
    target_link_libraries(kdeclarative ${QT_QTSCRIPTTOOLS_LIBRARY})
endif()

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

generate_export_header(kdeclarative)

install(
    FILES
    kdeclarative.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdeclarative_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(
    TARGETS kdeclarative
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)
