MACRO(PLASMA_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_test(plasma-${_testname} ${_testname}.cpp)
        target_link_libraries(plasma-${_testname}
            ${QT_QTTEST_LIBRARY}
            ${QT_QTDECLARATIVE_LIBRARY}
            ${KDE4_KIO_LIBS}
            ${KDE4_KDEUI_LIBS}
            plasma
        )
        if(QT_QTOPENGL_FOUND)
            target_link_libraries(plasma-${_testname} ${QT_QTOPENGL_LIBRARY})
        endif()
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
    ${QT_QTTEST_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${QT_QTSQL_LIBRARY}
    ${KDE4_KIO_LIBS}
    plasma
)

if(QT_QTOPENGL_FOUND)
    target_link_libraries(plasma-storagetest ${QT_QTOPENGL_LIBRARY})
endif()
