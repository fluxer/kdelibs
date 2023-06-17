include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_manual_test(kpasswdstore-manual kpasswdstoretest.cpp)
target_link_libraries(kpasswdstore-manual ${QT_QTTEST_LIBRARY} kdecore kpasswdstore)

kde4_add_manual_test(kpasswdroulettedialog-manual
    kpasswdroulettedialog-manual.cpp
)
target_link_libraries(kpasswdroulettedialog-manual kpasswdstore)
