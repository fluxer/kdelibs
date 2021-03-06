project(kioslave-http)

macro_optional_find_package(GSSAPI)
set_package_properties(GSSAPI PROPERTIES
    DESCRIPTION "Allows KIO to make use of certain HTTP authentication services"
    URL "http://web.mit.edu/kerberos/www"
    TYPE OPTIONAL
    PURPOSE "A MIT or HEIMDAL flavor of GSSAPI can be used"
)

if(GSSAPI_FOUND)
    set(HAVE_LIBGSSAPI 1)
    if(GSSAPI_FLAVOR STREQUAL "MIT")
        set(GSSAPI_MIT 1)
    else(GSSAPI_FLAVOR STREQUAL "MIT")
        set(GSSAPI_MIT 0)
    endif(GSSAPI_FLAVOR STREQUAL "MIT")

    include_directories(${GSSAPI_INCS})
else(GSSAPI_FOUND)
    set(HAVE_LIBGSSAPI 0)
    set(GSSAPI_MIT 0)
endif(GSSAPI_FOUND)

configure_file(
    config-gssapi.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-gssapi.h
)

include_directories(
    ${KDE4_KIO_INCLUDES}
    ${ZLIB_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/solid
    ${CMAKE_BINARY_DIR}/solid
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/kio/httpfilter
)

add_subdirectory(kcookiejar)
if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

########### next target ###############

add_executable(kio_http_cache_cleaner http_cache_cleaner.cpp)

target_link_libraries(kio_http_cache_cleaner
    ${KDE4_KIO_LIBS}
    ${ZLIB_LIBRARY}
)

install(TARGETS kio_http_cache_cleaner DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR})

########### next target ###############

set(kio_http_PART_SRCS
    http.cpp
    httpauthentication.cpp
    ${httpfilter_STAT_SRCS}
    ${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc
)

kde4_add_plugin(kio_http ${kio_http_PART_SRCS})

target_link_libraries(kio_http
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    kntlm
    ${ZLIB_LIBRARY}
)
if(GSSAPI_FOUND)
    target_link_libraries(kio_http ${GSSAPI_LIBS})
endif()

install(TARGETS kio_http DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### install files ###############

install(
    FILES
    http_cache_cleaner.desktop
    http.protocol
    https.protocol
    webdav.protocol
    webdavs.protocol
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
