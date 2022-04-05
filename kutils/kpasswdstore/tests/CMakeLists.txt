include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

kde4_add_manual_test(kpasswdstore-manual
    kpasswdstore-manual.cpp
)
target_link_libraries(kpasswdstore-manual ${KDE4_KPASSWDSTORE_LIBS})

kde4_add_manual_test(kpasswdroulettedialog-manual
    kpasswdroulettedialog-manual.cpp
)
target_link_libraries(kpasswdroulettedialog-manual ${KDE4_KPASSWDSTORE_LIBS})
