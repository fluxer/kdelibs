include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

kde4_add_test(kspeech-kspeechtest
    kspeechtest.cpp
)
target_link_libraries(kspeech-kspeechtest ${KDE4_KSPEECH_LIBS} ${QT_QTTEST_LIBRARY})
