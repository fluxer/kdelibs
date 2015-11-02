# This can be used for finding data files in the source dir, without installing them
project(knotifytest)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../config)

########### knotifytest ###############

set(knotifytest_SRCS main.cpp knotifytestwindow.cpp)

kde4_add_manual_test(knotifytest ${knotifytest_SRCS})

target_link_libraries(knotifytest ${KDE4_KIO_LIBS} knotifyconfig)
