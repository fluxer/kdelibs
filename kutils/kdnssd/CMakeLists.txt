if(AVAHI_FOUND)
    include_directories(${AVAHI_INCLUDE_DIR})
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51006)

set(kdnssd_LIB_SRCS
    kdnssd.cpp
)

add_library(kdnssd SHARED ${kdnssd_LIB_SRCS})

target_link_libraries(kdnssd PUBLIC
    ${QT_QTCORE_LIBRARY}
    kdecore
)

if(AVAHI_FOUND)
    target_link_libraries(kdnssd PRIVATE ${AVAHI_LIBRARIES})
endif()

set_target_properties(kdnssd PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(kdnssd)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kdnssd_export.h
    kdnssd.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    TARGETS kdnssd
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)
