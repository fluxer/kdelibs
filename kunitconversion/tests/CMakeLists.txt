include_directories(${KDE4_KDECORE_INCLUDES} ${KDE4_KDEUI_INCLUDES})

macro(KUNITCONVERSION_UNIT_TESTS)
    foreach(_testname ${ARGN})
        kde4_add_test(kunitconversion-${_testname} ${_testname}.cpp)
        target_link_libraries(kunitconversion-${_testname}
            ${KDE4_KDECORE_LIBS}
            ${KDE4_KUNITCONVERSION_LIBS}
            ${QT_QTTEST_LIBRARY}
            ${QT_QTGUI_LIBRARY}
        )
    endforeach(_testname)
endmacro(KUNITCONVERSION_UNIT_TESTS)

KUNITCONVERSION_UNIT_TESTS(
    valuetest
    categorytest
    convertertest
)
