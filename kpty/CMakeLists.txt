project(kpty)

include_directories(${KDE4_KDECORE_INCLUDES})

set(kpty_LIB_SRCS
    kpty.cpp
    kptydevice.cpp
    kptyprocess.cpp
)

add_library(kpty SHARED ${kpty_LIB_SRCS})

target_link_libraries(kpty PRIVATE
    ${UTIL_LIBRARY}
)

target_link_libraries(kpty PUBLIC
    kdecore
)

set_target_properties(kpty PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(
    TARGETS kpty
    EXPORT kdelibsLibraryTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

generate_export_header(kpty)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kpty_export.h
    kpty.h
    kptydevice.h
    kptyprocess.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

########### next target ###############

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
