project(kpty)

include_directories(${KDE4_KDECORE_INCLUDES})

set(kpty_LIB_SRCS
    kpty.cpp
    kptydevice.cpp
    kptyprocess.cpp
)

add_library(kpty ${LIBRARY_TYPE} ${kpty_LIB_SRCS})

target_link_libraries(kpty PRIVATE
    ${UTIL_LIBRARY}
)

target_link_libraries(kpty PUBLIC
    ${KDE4_KDECORE_LIBS}
)

set_target_properties(kpty PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(
    TARGETS kpty
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

generate_export_header(kpty)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kpty_export.h
    kpty.h
    kptydevice.h
    kptyprocess.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

########### next target ###############

if(NOT HAVE_OPENPTY)
    add_executable(kgrantpty kgrantpty.c)
    target_link_libraries(kgrantpty ${KDE4_KDECORE_LIBS})
    install(TARGETS kgrantpty DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR})
endif()

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
