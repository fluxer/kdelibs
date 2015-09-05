include_directories(${CMAKE_SOURCE_DIR} ../ ${CMAKE_CURRENT_BINARY_DIR})

kde4_add_manual_test(kemoticonstest main.cpp)
target_link_libraries(kemoticonstest ${KDE4_KDEUI_LIBS} kemoticons)

add_definitions(-DSRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/")
set(kemoticontest2_SRCS kemoticontest.cpp)
kde4_add_test(kutils-kemoticontest2 ${kemoticontest2_SRCS})
target_link_libraries(kutils-kemoticontest2  ${QT_QTTEST_LIBRARY} ${KDE4_KIO_LIBS} kemoticons)
