include_directories(
    ${CMAKE_SOURCE_DIR}/khtml/java
    ${CMAKE_SOURCE_DIR}/khtml
)


########### next target ###############

set(testserver_SRCS
    testkjavaappletserver.cpp
    ../kjavaappletcontext.cpp
    ../kjavaappletviewer.cpp
    ../kjavaprocess.cpp
    ../kjavaapplet.cpp
    ../kjavaappletwidget.cpp
    ../kjavadownloader.cpp
    ../kjavaappletserver.cpp
)

kde4_add_manual_test(khtml-testserver ${testserver_SRCS})

target_link_libraries(khtml-testserver
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KDEUI_LIBS}
)
