if(SPEECHD_FOUND)
    include_directories(${SPEECHD_INCLUDE_DIR})
    add_definitions(-DHAVE_SPEECHD)
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51008)

set(kspeech_LIB_SRCS
    kspeech.cpp
)

add_library(kspeech ${LIBRARY_TYPE} ${kspeech_LIB_SRCS})

target_link_libraries(kspeech PUBLIC
    ${KDE4_KDECORE_LIBS}
)

if(SPEECHD_FOUND)
    target_link_libraries(kspeech PRIVATE ${SPEECHD_LIBRARIES})
endif()

set_target_properties(kspeech PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(kspeech)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kspeech_export.h
    kspeech.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(
    TARGETS kspeech
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
