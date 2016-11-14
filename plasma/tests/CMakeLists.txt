MACRO(PLASMA_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_test(plasma-${_testname} ${_testname}.cpp)
        target_link_libraries(plasma-${_testname}
            ${KDE4_KIO_LIBS}
            ${KDE4_KDEUI_LIBS}
            ${KDE4_PLASMA_LIBS}
            ${QT_QTTEST_LIBRARY}
            ${QT_QTDECLARATIVE_LIBRARY}
        )
    ENDFOREACH(_testname)
ENDMACRO(PLASMA_UNIT_TESTS)

PLASMA_UNIT_TESTS(
    packagestructuretest
    packagemetadatatest
    plasmoidpackagetest
    runnercontexttest
    configloadertest
)

set(storatetest_SRCS
    storagetest.cpp
    ../private/storage.cpp
    ../private/storagethread.cpp
)

kde4_add_test(plasma-storagetest ${storatetest_SRCS})
target_link_libraries(plasma-storagetest
    ${KDE4_KIO_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${QT_QTSQL_LIBRARY}
)
