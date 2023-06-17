include_directories(${KDE4_KPTY_INCLUDES})

kde4_add_test(kptyprocesstest kptyprocesstest.cpp)

target_link_libraries(kptyprocesstest ${QT_QTTEST_LIBRARY} kpty)
