if(EXIV2_FOUND)
    include_directories(${EXIV2_INCLUDE_DIR})
    add_definitions(-DHAVE_EXIV2 ${EXIV2_DEFINITIONS})
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51003)

set(kexiv2_LIB_SRCS
    kexiv2.cpp
)

add_library(kexiv2 ${LIBRARY_TYPE} ${kexiv2_LIB_SRCS})

target_link_libraries(kexiv2 PUBLIC
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
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
    COMPONENT Devel
)

install(
    TARGETS kexiv2
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()