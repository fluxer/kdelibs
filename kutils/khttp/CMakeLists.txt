if(LIBMICROHTTPD_FOUND)
    include_directories(${LIBMICROHTTPD_INCLUDE_DIR})
    add_definitions(-DHAVE_LIBMICROHTTPD)
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51007)

set(khttp_LIB_SRCS
    khttp.cpp
)

add_library(khttp ${LIBRARY_TYPE} ${khttp_LIB_SRCS})

target_link_libraries(khttp PUBLIC
    ${KDE4_KDECORE_LIBS}
    ${QT_QTNETWORK_LIBRARY}
)

if(LIBMICROHTTPD_FOUND)
    target_link_libraries(khttp PRIVATE ${LIBMICROHTTPD_LIBRARIES})
endif()

set_target_properties(khttp PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(khttp)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/khttp_export.h
    khttp.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(
    TARGETS khttp
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)
