include_directories(
    ${CMAKE_SOURCE_DIR}/kutils/kpasswdstore
    ${CMAKE_BINARY_DIR}/kutils/kpasswdstore
    ${CURL_INCLUDE_DIRS}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51010)

set(kemail_LIB_SRCS
    kemailsettings.cpp
    kemail.cpp
    kemaildialog.cpp
)

add_library(kemail SHARED ${kemail_LIB_SRCS})

target_link_libraries(kemail PUBLIC
    kdecore
    kdeui
    kpasswdstore
)
target_link_libraries(kemail PRIVATE ${CURL_LIBRARIES})

set_target_properties(kemail PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(kemail)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kemail_export.h
    kemail.h
    kemaildialog.h
    kemailsettings.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    TARGETS kemail
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)
