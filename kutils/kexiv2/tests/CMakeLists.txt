include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

kde4_add_test(kexiv2-kexiv2test
    kexiv2test.cpp
)
target_link_libraries(kexiv2-kexiv2test ${QT_QTTEST_LIBRARY} kexiv2)
