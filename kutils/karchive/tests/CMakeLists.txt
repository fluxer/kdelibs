include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

kde4_add_test(karchive-karchivetest
    karchivetest.cpp
)
target_link_libraries(karchive-karchivetest ${QT_QTTEST_LIBRARY} karchive)

kde4_add_test(karchive-kcompressortest
    kcompressortest.cpp
)
target_link_libraries(karchive-kcompressortest ${QT_QTTEST_LIBRARY} karchive)

kde4_add_test(karchive-kdecompressortest
    kdecompressortest.cpp
)
target_link_libraries(karchive-kdecompressortest ${QT_QTTEST_LIBRARY} karchive)
