include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_manual_test(kpasswdstore-manual kpasswdstoretest.cpp)
target_link_libraries(kpasswdstore-manual ${KDE4_KDECORE_LIBS} ${KDE4_KPASSWDSTORE_LIBS} ${QT_QTTEST_LIBRARY})

kde4_add_manual_test(kpasswdroulettedialog-manual
    kpasswdroulettedialog-manual.cpp
)
target_link_libraries(kpasswdroulettedialog-manual ${KDE4_KPASSWDSTORE_LIBS})
