if(EXIV2_FOUND)
    include_directories(${EXIV2_INCLUDE_DIR})
    add_definitions(${EXIV2_DEFINITIONS})
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51003)

set(kexiv2_LIB_SRCS
    kexiv2.cpp
)

add_library(kexiv2 SHARED ${kexiv2_LIB_SRCS})

target_link_libraries(kexiv2 PUBLIC
    kdecore
    kdeui
)

if(EXIV2_FOUND)
    target_link_libraries(kexiv2 PRIVATE ${EXIV2_LIBRARIES})
endif()

set_target_properties(kexiv2 PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

set_target_properties(kexiv2 PROPERTIES
    COMPILE_FLAGS "${KDE4_ENABLE_EXCEPTIONS}"
)

generate_export_header(kexiv2)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kexiv2_export.h
    kexiv2.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    TARGETS kexiv2
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()