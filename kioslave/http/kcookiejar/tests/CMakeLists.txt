########### next target ###############

set(kcookiejartest_SRCS kcookiejartest.cpp)

kde4_add_test(kcookiejar-kcookiejartest ${kcookiejartest_SRCS})

target_link_libraries(kcookiejar-kcookiejartest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})
