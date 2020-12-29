include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_KDECORE_INCLUDES}
)

add_subdirectory(solid)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
