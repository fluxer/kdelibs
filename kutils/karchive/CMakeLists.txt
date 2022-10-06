if(LibArchive_FOUND)
    include_directories(${LibArchive_INCLUDE_DIRS})
    add_definitions(-DHAVE_LIBARCHIVE)
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51009)

set(karchive_LIB_SRCS
    karchive.cpp
)

add_library(karchive ${LIBRARY_TYPE} ${karchive_LIB_SRCS})

target_link_libraries(karchive PUBLIC
    ${KDE4_KDECORE_LIBS}
)

if(LibArchive_FOUND)
    target_link_libraries(karchive PRIVATE ${LibArchive_LIBRARIES})
endif()

set_target_properties(karchive PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(karchive)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/karchive_export.h
    karchive.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(
    TARGETS karchive
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
