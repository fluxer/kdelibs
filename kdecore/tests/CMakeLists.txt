include_directories( ${KDE4_KDECORE_INCLUDES} )
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

MACRO(KDECORE_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_test(kdecore-${_testname} ${_testname}.cpp)
        target_link_libraries(kdecore-${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
    ENDFOREACH(_testname)
ENDMACRO(KDECORE_UNIT_TESTS)
MACRO(KDECORE_EXECUTABLE_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_manual_test(kdecore-${_testname} ${_testname}.cpp)
        target_link_libraries(kdecore-${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})
    ENDFOREACH(_testname)
ENDMACRO(KDECORE_EXECUTABLE_TESTS)


########### next target ###############

KDECORE_UNIT_TESTS(
    klocaletimeformattest
    klocalizedstringtest
    kmountpointtest
    kstandarddirstest
    kaboutdatatest
    kurltest
    kstringhandlertest
    cplusplustest
    ksortablelisttest
    kcharsetstest
    # FIXME: utterly broken right now
    # kcalendartest
    kmacroexpandertest
    kshelltest
    ktimezonestest
    kconfigtest
    kurlmimetest
    klockfiletest
    ktempdirtest
    ksharedptrtest
    kdatetimetest
    ksavefiletest
    kdesktopfiletest
    ktemporaryfiletest
    kautostarttest
    kjobtest
    ksycocadicttest
    kservicetest
    kglobalstatictest
    kglobaltest
    globalcleanuptest
    kprocesstest
    kconfigafterkglobaltest1
    kconfigafterkglobaltest2
    ksycocathreadtest
    qcoreapptest
    kunitconversiontest
    kdevicedatabasetest
    kdebugtest
)

KDECORE_EXECUTABLE_TESTS(
    kcmdlineargstest
    dbuscalltest
    startserviceby
    klockfile_testlock # helper for klockfiletest
)

########### kentrymaptest ###############

# compile KEntryMap into the test since it's not exported
set(kentrymaptest_SRCS kentrymaptest.cpp ../config/kconfigdata.cpp)
kde4_add_test(kdecore-kentrymaptest ${kentrymaptest_SRCS})
target_link_libraries(kdecore-kentrymaptest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )

########### klocaletest ###############

# compile into the test since it's not exported
set(klocaletest_SRCS klocaletest.cpp ../date/kdayperiod.cpp)
kde4_add_test(kdecore-klocaletest ${klocaletest_SRCS})
target_link_libraries(kdecore-klocaletest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )

########### kdatetimeformattertest ###############

# compile KDateTimeFormatter and KDayPeriod into the test since it's not exported
set(kdatetimeformattertest_SRCS kdatetimeformattertest.cpp ../date/kdatetimeformatter.cpp ../date/kdayperiod.cpp)
kde4_add_test(kdecore-kdatetimeformattertest ${kdatetimeformattertest_SRCS})
target_link_libraries(kdecore-kdatetimeformattertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )

########### kmimetypetest ###############

# compile kmimemagicrule.cpp into the test since it's not exported and we call match().
set(kmimetypetest_SRCS kmimetypetest.cpp ../services/kmimemagicrule.cpp)
kde4_add_test(kdecore-kmimetypetest ${kmimetypetest_SRCS})
target_link_libraries(kdecore-kmimetypetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

########### kmimeglobsfileparsertest ###############

# compile kmimeglobsfileparser.cpp into the test since it's not exported
set(kmimeglobsfileparsertest_SRCS kmimeglobsfileparsertest.cpp ../services/kmimeglobsfileparser.cpp)
kde4_add_test(kdecore-kmimeglobsfileparsertest ${kmimeglobsfileparsertest_SRCS})
target_link_libraries(kdecore-kmimeglobsfileparsertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

########### module for klibloadertest4 ###############

set(klibloadertestmodule4_PART_SRCS klibloadertest4_module.cpp )

kde4_add_plugin(kdecore-klibloadertestmodule4 ${klibloadertestmodule4_PART_SRCS})

target_link_libraries(kdecore-klibloadertestmodule4  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

########### kcompositejobtest ###############

set(kcompositejobtest_SRCS kcompositejobtest.cpp)
kde4_add_test(kdecore-kcompositejobtest ${kcompositejobtest_SRCS})
target_link_libraries(kdecore-kcompositejobtest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})
