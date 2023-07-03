# FIXME: they are bogus
# add_subdirectory(kconfig_compiler)

include_directories( ${KDE4_KDEUI_INCLUDES} )

MACRO(KDEUI_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        if(EXISTS ${_testname}.h)
            kde4_add_test(kdeui-${_testname} ${_testname}.cpp ${_testname}.h)
        else()
            kde4_add_test(kdeui-${_testname} ${_testname}.cpp)
        endif()
        target_link_libraries(kdeui-${_testname}
            ${QT_QTTEST_LIBRARY}
            ${QT_QTXML_LIBRARY}
            kdeui
        )
    ENDFOREACH(_testname)
ENDMACRO(KDEUI_UNIT_TESTS)
MACRO(KDEUI_EXECUTABLE_TESTS)
    FOREACH(_testname ${ARGN})
        if(EXISTS ${_testname}.h)
            kde4_add_manual_test(kdeui-${_testname} ${_testname}.cpp ${_testname}.h)
        else()
            kde4_add_manual_test(kdeui-${_testname} ${_testname}.cpp)
        endif()
        target_link_libraries(kdeui-${_testname}
            ${QT_QTTEST_LIBRARY}
            ${QT_QTXML_LIBRARY}
            kdeui
        )
    ENDFOREACH(_testname)
ENDMACRO(KDEUI_EXECUTABLE_TESTS)

KDEUI_UNIT_TESTS(
    kactioncollectiontest
    kbuttongrouptest
    kcompletioncoretest
    kconfigskeletontest
    kdualactiontest
    kfindtest
    kglobalsettingstest
    kmainwindow_unittest
    klineedit_unittest
    kcombobox_unittest
    kdialog_unittest
    kreplacetest
    kshortcuttest
    kstandardshortcuttest
    kuniqueapptest
    kstartupinfo_unittest
    kcolorutilstest
    kxmlgui_unittest
    ktimezonewidget_unittest
    kiconloader_unittest
    ktabwidget_unittest
    ktoolbar_unittest
    krichtextedittest
    kselectaction_unittest
    klistwidgetsearchlinetest
    ktimecomboboxtest
    kdatecomboboxtest
    kdatetimeedittest
    kconfigdialog_unittest
    kcolorbuttontest
    kglobalshortcuttest
    klinkitemselectionmodeltest
    kstandardactiontest
    ktextedit_unittest
    kactioncategorytest
    kapplication_unittest
    kconfigguitest
)

KDEUI_EXECUTABLE_TESTS(
    kglobalsettingsclient # helper program for kglobalsettingstest
    kwindowtest
    fixx11h_test
    fixx11h_test2
)
