include_directories(${KDE4_KPTY_INCLUDES})

kde4_add_test(kptyprocesstest kptyprocesstest.cpp)
target_link_libraries(kptyprocesstest ${KDE4_KPTY_LIBS} ${QT_QTTEST_LIBRARY})
