include_directories(${KDE4_KPARTS_INCLUDES})

########### next target ###############

set(kpartstest_SRCS
    testmainwindow.cpp
    parts.cpp
    notepad.cpp
)

kde4_add_manual_test(kparts-kpartstest ${kpartstest_SRCS})

target_link_libraries(kparts-kpartstest ${KDE4_KPARTS_LIBS})

########### next target ###############

set(normalktmtest_SRCS
    normalktm.cpp
    parts.cpp
    notepad.cpp
)

kde4_add_manual_test(kparts-normalktmtest ${normalktmtest_SRCS})

target_link_libraries(kparts-normalktmtest ${KDE4_KPARTS_LIBS})

########### next target ###############

kde4_add_manual_test(kparts-partviewer partviewer.cpp)

target_link_libraries(kparts-partviewer ${KDE4_KPARTS_LIBS})

########### test prog for "open or save question" ###############

kde4_add_manual_test(kparts-openorsavequestion openorsavequestion.cpp)

target_link_libraries(kparts-openorsavequestion ${KDE4_KPARTS_LIBS})

########### next target ###############

kde4_add_plugin(spellcheckplugin plugin_spellcheck.cpp)

target_link_libraries(spellcheckplugin ${KDE4_KPARTS_LIBS})

install(TARGETS spellcheckplugin DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

kde4_add_plugin(notepadpart notepad.cpp)

target_link_libraries(notepadpart ${KDE4_KPARTS_LIBS})

target_compile_definitions(notepadpart PRIVATE -DKDESRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/")

install(TARGETS notepadpart DESTINATION ${PLUGIN_INSTALL_DIR})

########### unit tests ###############

MACRO(KPARTS_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_test(kparts-${_testname} ${_testname}.cpp)
        target_link_libraries(kparts-${_testname} ${KDE4_KPARTS_LIBS} ${QT_QTTEST_LIBRARY})
    ENDFOREACH(_testname)
ENDMACRO(KPARTS_UNIT_TESTS)
MACRO(KPARTS_EXECUTABLE_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_manual_test(kparts-${_testname} ${_testname}.cpp)
        target_link_libraries(kparts-${_testname} ${KDE4_KPARTS_LIBS} ${QT_QTTEST_LIBRARY})
    ENDFOREACH(_testname)
ENDMACRO(KPARTS_EXECUTABLE_TESTS)

KPARTS_UNIT_TESTS(
    parttest
    openorsavequestion_unittest
)

########### install files ###############

# Normally this would be needed. For the test programs we can avoid that, see addResourceDir.
#install( FILES notepadpart.rc  DESTINATION  ${DATA_INSTALL_DIR}/notepadpart )
#install( FILES partviewer_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/partviewer )
#install( FILES kpartstest_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/kpartstest )
#install( FILES kpartstest_part1.rc  DESTINATION  ${DATA_INSTALL_DIR}/kpartstestpart )

# To test plugins without installing them, try "ln -s kpartplugins <srcdir>"
#install( FILES plugin_foobar.rc plugin_spellcheck.rc  DESTINATION  ${DATA_INSTALL_DIR}/notepadpart/kpartplugins )
