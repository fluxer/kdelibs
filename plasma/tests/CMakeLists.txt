MACRO(PLASMA_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_test(plasma-${_testname} ${_testname}.cpp)
        target_link_libraries(plasma-${_testname}
            ${QT_QTTEST_LIBRARY}
            ${QT_QTDECLARATIVE_LIBRARY}
            kio
            kdeui
            plasma
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
