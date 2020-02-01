include_directories(
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_KDECORE_INCLUDES}
    ../Weaver
)

########### next target ###############

set(JobTests_SRCS
    JobTests.cpp
    AppendCharacterJob.cpp
    AppendCharacterAndVerifyJob.cpp
)

kde4_add_test(ThreadWeaverJobTests ${JobTests_SRCS})

target_link_libraries(ThreadWeaverJobTests
    ${KDE4_THREADWEAVER_LIBS}
    ${QT_QTTEST_LIBRARY}
)

########### next target ###############

set(QueueTests_SRCS
    QueueTests.cpp
    AppendCharacterJob.cpp
)

kde4_add_test(ThreadWeaverQueueTests ${QueueTests_SRCS})

target_link_libraries(ThreadWeaverQueueTests
    ${KDE4_THREADWEAVER_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
)

########### next target ###############

set(DeleteTest_SRCS
    DeleteTest.cpp
    AppendCharacterJob.cpp
)

kde4_add_test(ThreadWeaverDeleteTest ${DeleteTest_SRCS})

target_link_libraries(ThreadWeaverDeleteTest
    ${KDE4_THREADWEAVER_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
)
