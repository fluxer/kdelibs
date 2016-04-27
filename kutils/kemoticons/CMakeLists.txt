# for the export headers
include_directories(${CMAKE_BINARY_DIR}/kutils)

add_subdirectory(providers)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
