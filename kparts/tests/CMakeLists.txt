include_directories(${KDE4_KPARTS_INCLUDES})

########### unit tests ###############

MACRO(KPARTS_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_test(kparts-${_testname} ${_testname}.cpp)
        target_link_libraries(kparts-${_testname} ${QT_QTTEST_LIBRARY} kparts)
    ENDFOREACH(_testname)
ENDMACRO(KPARTS_UNIT_TESTS)

KPARTS_UNIT_TESTS(
    parttest
)

