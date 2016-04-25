include_directories(
    ${CMAKE_SOURCE_DIR}
    ../
    ${CMAKE_CURRENT_BINARY_DIR}
)

kde4_add_manual_test(kutils-kemoticonstest main.cpp)
target_link_libraries(kutils-kemoticonstest ${KDE4_KDEUI_LIBS} kemoticons)

set(kemoticontest2_SRCS kemoticontest.cpp)
kde4_add_test(kutils-kemoticontest2 ${kemoticontest2_SRCS})
target_link_libraries(kutils-kemoticontest2
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KIO_LIBS}
    kemoticons
)
