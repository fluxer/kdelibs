if(LibArchive_FOUND)
    include_directories(${LibArchive_INCLUDE_DIRS})
endif()

if(BZIP2_FOUND)
    include_directories(${BZIP2_INCLUDE_DIR})
endif()

if(LIBLZMA_FOUND)
    include_directories(${LIBLZMA_INCLUDE_DIRS})
endif()

include_directories(${LIBDEFLATE_INCLUDES})
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51009)

set(karchive_LIB_SRCS
    karchive.cpp
    kcompressor.cpp
    kdecompressor.cpp
)

add_library(karchive SHARED ${karchive_LIB_SRCS})

target_link_libraries(karchive PUBLIC
    ${KDE4_KDECORE_LIBS}
)
target_link_libraries(karchive PRIVATE
    ${LIBDEFLATE_LIBRARIES}
)

if(LibArchive_FOUND)
    target_link_libraries(karchive PRIVATE ${LibArchive_LIBRARIES})
endif()

if(BZIP2_FOUND)
    target_link_libraries(karchive PRIVATE ${BZIP2_LIBRARIES})
endif()

if(LIBLZMA_FOUND)
    target_link_libraries(karchive PRIVATE ${LIBLZMA_LIBRARIES})
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
    kcompressor.h
    kdecompressor.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    TARGETS karchive
    EXPORT kdelibsLibraryTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
