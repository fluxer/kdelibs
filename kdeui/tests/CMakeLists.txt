# FIXME: they are bogus
# add_subdirectory(kconfig_compiler)

include_directories( ${KDE4_KDEUI_INCLUDES} proxymodeltestsuite )

MACRO(KDEUI_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        if(EXISTS ${_testname}.h)
            kde4_add_test(kdeui-${_testname} ${_testname}.cpp ${_testname}.h)
        else()
            kde4_add_test(kdeui-${_testname} ${_testname}.cpp)
        endif()
        target_link_libraries(kdeui-${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES})
    ENDFOREACH(_testname)
ENDMACRO(KDEUI_UNIT_TESTS)
MACRO(KDEUI_EXECUTABLE_TESTS)
    FOREACH(_testname ${ARGN})
        if(EXISTS ${_testname}.h)
            kde4_add_manual_test(kdeui-${_testname} ${_testname}.cpp ${_testname}.h)
        else()
            kde4_add_manual_test(kdeui-${_testname} ${_testname}.cpp)
        endif()
        target_link_libraries(kdeui-${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN_LIBRARIES})
        set_target_properties(kdeui-${_testname} PROPERTIES COMPILE_FLAGS -DKDESRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}\\"")
    ENDFOREACH(_testname)
ENDMACRO(KDEUI_EXECUTABLE_TESTS)

SET(proxyModelTestSources
    kselectionproxymodeltestsuite.cpp
)

add_subdirectory(proxymodeltestsuite)

MACRO(KDEUI_PROXYMODEL_TESTS)
    FOREACH(_testname ${ARGN})
        if(EXISTS ${_testname}.h)
            kde4_add_test(kdeui-${_testname} ${_testname}.cpp ${_testname}.h ${proxyModelTestSources})
        else()
            kde4_add_test(kdeui-${_testname} ${_testname}.cpp ${proxyModelTestSources})
        endif()
        target_link_libraries(kdeui-${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} ${KDEWIN32_LIBRARIES} proxymodeltestsuite)
    ENDFOREACH(_testname)
ENDMACRO(KDEUI_PROXYMODEL_TESTS)


KDEUI_UNIT_TESTS(
    kactioncollectiontest
    kbuttongrouptest
    kcompletioncoretest
    kconfigskeletontest
    kdualactiontest
    kfadewidgeteffecttest
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
    kwordwraptest
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

KDEUI_PROXYMODEL_TESTS(
    kdescendantsproxymodeltest
    kselectionproxymodeltest
    testmodelqueuedconnections
)

KDEUI_EXECUTABLE_TESTS(
    kaccelgentest
    kactionselectortest
    kanimatedbuttontest
    kcharselecttest
    kapptest
    kassistantdialogtest
    kbugreporttest
    kcategorizedviewtest
    kcodecactiontest
    kcolorcollectiontest
    kcolordlgtest
    kcolorcombotest
    kcomboboxtest
    kdatepicktest
    klanguagebuttontest
    kdatetabletest
    kdatetimewidgettest
    kdatewidgettest
    kdebugtest_gui
    kdialogbuttonboxtest
    kdialogtest
    kfontdialogtest
    kglobalsettingsclient # helper program for kglobalsettingstest
    khboxtest
    kiconeffecttest
    kiconloadertest
    kinputdialogtest
    kjobtrackerstest
    kledtest
    klineedittest
    kmessageboxtest
    kmessagetest
    kmessagewidgettest
    knewpassworddialogtest
    kstatusnotifieritemtest
    knotificationrestrictionstest
    knuminputtest
    kpagedialogtest
    kpagewidgettest
    kpassivepopuptest
    kpassworddialogtest
    kpixmapregionselectordialogtest
    kpopuptest
    kprogressdialogtest
    krulertest
    kselectactiontest
    kseparatortest
    kstatusbartest
    ksqueezedtextlabeltest
    ksystemtraytest
    ktabwidgettest
    ktextedittest
    ktextbrowsertest
    ktitlewidgettest
    ktoolbartest
    ktoolbarlabelactiontest
    kwallettest
    kwidgetitemdelegatetest
    kwindowtest
    kxmlguitest
    kxmlguiwindowtest
    testqtargs
    kpixmapsequenceoverlaypaintertest
    ktreewidgetsearchlinetest
    kcompletionuitest
    kmainwindowrestoretest
    kmainwindowtest
    kmodifierkeyinfotest
)

if (Q_WS_X11)
    KDEUI_EXECUTABLE_TESTS(
        fixx11h_test
        fixx11h_test2
        kxerrorhandlertest
        kmanagerselectiontest
    )
    target_link_libraries(kdeui-kmanagerselectiontest ${X11_X11_LIB})
    target_link_libraries(kdeui-kxerrorhandlertest ${X11_X11_LIB})
endif (Q_WS_X11)

## kplottest

set(kplottest_SRCS testplot_widget.cpp testplot_main.cpp)
kde4_add_manual_test(kplottest ${kplottest_SRCS})
target_link_libraries(kplottest ${KDE4_KDEUI_LIBS})

## kcolorutilsdemo

SET(kcolorUtilsDemoSources kcolorutilsdemo.cpp kimageframe.cpp ../colors/kcolorspaces.cpp kcolorutilsdemo.ui)
kde4_add_manual_test(kcolorutilsdemo ${kcolorUtilsDemoSources})
target_link_libraries(kcolorutilsdemo ${KDE4_KDEUI_LIBS} ${KDEWIN_LIBRARIES})

add_subdirectory(proxymodeltestapp)

