include_directories(${KDE4_KIO_INCLUDES})

set(krosstest_SRCS
    testobject.cpp
    main.cpp
)

kde4_add_manual_test(krosstest ${krosstest_SRCS})

target_link_libraries(krosstest ${KDE4_KDEUI_LIBS} krosscore)
