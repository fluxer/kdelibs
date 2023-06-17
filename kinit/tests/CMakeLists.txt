include_directories(${KDE4_KIO_INCLUDES})

kde4_add_manual_test(klaunchertest klaunchertest.cpp)

target_link_libraries(klaunchertest kdecore kio)
