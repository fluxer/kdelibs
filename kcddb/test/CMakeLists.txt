include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/../libkcddb
)


MACRO(KCDDB_EXECUTABLE_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_manual_test(kcddb-${_testname} ${_testname}.cpp)
        target_link_libraries(kcddb-${_testname}
            ${KDE4_KDEUI_LIBS}
            ${KDE4_KCDDB_LIBS}
            ${QT_QTTEST_LIBRARY}
        )
    ENDFOREACH(_testname)
ENDMACRO(KCDDB_EXECUTABLE_TESTS)
MACRO(KCDDB_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_test(kcddb-${_testname} ${_testname}.cpp)
        target_link_libraries(kcddb-${_testname}
            ${KDE4_KDEUI_LIBS}
            ${KDE4_KCDDB_LIBS}
            ${QT_QTTEST_LIBRARY}
        )
    ENDFOREACH(_testname)
ENDMACRO(KCDDB_UNIT_TESTS)

KCDDB_UNIT_TESTS(
    asynchttplookuptest
    asynccddblookuptest
    synccddblookuptest
    synchttplookuptest
    utf8test
    musicbrainztest
    # FIXME: this test hangs
    # asyncmusicbrainztest
    cdinfotest
    cachetest
    musicbrainztest-severaldiscs
    musicbrainztest-fulldate
)

KCDDB_EXECUTABLE_TESTS(
    syncsmtpsubmittest
    asyncsmtpsubmittest
    asynchttpsubmittest
    synchttpsubmittest
    sitestest
)

