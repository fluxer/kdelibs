if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    add_definitions(-DHAVE_OPENSSL)
endif()

# for the kded module
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51004)

set(kpasswdstore_LIB_SRCS
    kpasswdstore.cpp
    kpasswdroulettedialog.cpp
    kpasswdroulettedialog.ui
)

add_library(kpasswdstore ${LIBRARY_TYPE} ${kpasswdstore_LIB_SRCS})

target_link_libraries(kpasswdstore PUBLIC
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTDBUS_LIBRARY}
)

set_target_properties(kpasswdstore PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(kpasswdstore)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kpasswdstore_export.h
    kpasswdstore.h
    kpasswdroulettedialog.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(
    TARGETS kpasswdstore
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

add_subdirectory(kded)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
