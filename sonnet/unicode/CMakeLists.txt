project(parseucd)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${X11_INCLUDE_DIR}
)

add_executable(parseucd parseucd/parseucd.cpp)

target_link_libraries(parseucd
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
)
