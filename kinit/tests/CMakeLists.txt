include_directories( ${KDE4_KIO_INCLUDES} )

set(klaunchertest_SRCS klaunchertest.cpp )

kde4_add_manual_test(klaunchertest ${klaunchertest_SRCS})

target_link_libraries(klaunchertest  ${KDE4_KDECORE_LIBS} kio )
