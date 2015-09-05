MACRO(KFILE_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_test(kfile-${_testname} ${_testname}.cpp)
        target_link_libraries(kfile-${_testname}
            ${KDE4_KFILE_LIBS}
            ${KDE4_KIO_LIBS}
            ${QT_QTTEST_LIBRARY}
            ${QT_QTXML_LIBRARY}
        )
    ENDFOREACH(_testname)
ENDMACRO(KFILE_UNIT_TESTS)
MACRO(KFILE_EXECUTABLE_TESTS)
    FOREACH(_testname ${ARGN})
        KDE4_ADD_MANUAL_TEST(kfile-${_testname} ${_testname}.cpp)
        target_link_libraries(kfile-${_testname}
            ${KDE4_KFILE_LIBS}
            ${QT_QTTEST_LIBRARY}
        )
    ENDFOREACH(_testname)
ENDMACRO(KFILE_EXECUTABLE_TESTS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

KFILE_EXECUTABLE_TESTS(
    kdirselectdialogtest
    kfiletreeviewtest
)

add_definitions(
    -DTEST_DATA="\\"${CMAKE_SOURCE_DIR}/solid/solid/backends/fakehw/fakecomputer.xml\\""
)

KFILE_UNIT_TESTS(
    kfileplacesmodeltest
    kurlnavigatortest
    kdiroperatortest
    knewfilemenutest
)

