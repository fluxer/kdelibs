MACRO(KFILE_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_test(kfile-${_testname} ${_testname}.cpp)
        target_link_libraries(kfile-${_testname}
            ${QT_QTTEST_LIBRARY}
            ${QT_QTXML_LIBRARY}
            kfile
            kio
        )
    ENDFOREACH(_testname)
ENDMACRO(KFILE_UNIT_TESTS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_definitions(
    -DTEST_DATA="${CMAKE_SOURCE_DIR}/solid/solid/backends/fakehw/fakecomputer.xml"
)

KFILE_UNIT_TESTS(
    kfileplacesmodeltest
    kurlnavigatortest
    kdiroperatortest
    knewfilemenutest
)
