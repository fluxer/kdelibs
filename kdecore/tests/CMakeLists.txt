include_directories( ${KDE4_KDECORE_INCLUDES} )
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

MACRO(KDECORE_UNIT_TESTS)
    FOREACH(_testname ${ARGN})
        kde4_add_test(kdecore-${_testname} ${_testname}.cpp)
        target_link_libraries(kdecore-${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY})
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
    # FIXME: due to issues with kgzipfilter the test bellow is disabled as it
    # causes a hang
    # karchivetest
    kdirwatch_unittest
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
    kasciitest
    ktimezonestest
    kconfigtest
    kurlmimetest
    klockfiletest
    ktempdirtest
    ksharedptrtest
    kshareddatacachetest
    kdatetimetest
    ksavefiletest
    kautosavefiletest
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
    ktcpsockettest
    ksycocathreadtest
    kdebug_unittest
    kencodingdetectortest
    qcoreapptest
    kdebug_qcoreapptest
    kmimetype_nomimetypes
)

KDECORE_UNIT_TESTS(
    klocalsockettest
    klocalsocketservertest
)

KDECORE_EXECUTABLE_TESTS(
    kdirwatchtest
    krandomsequencetest
    ktartest
    kziptest
    kdebugtest
    kcmdlineargstest
    kmemtest
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

########### kdirwatchtest_gui ###############

add_executable(kdecore-kdirwatchtest_gui kdirwatchtest_gui.cpp)
target_link_libraries(kdecore-kdirwatchtest_gui ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY})

########### klimitediodevicetest ###############

kde4_add_test(kdecore-klimitediodevicetest klimitediodevicetest.cpp ../io/klimitediodevice.cpp)
target_link_libraries(kdecore-klimitediodevicetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

########### kmimetypetest ###############

# compile kmimemagicrule.cpp into the test since it's not exported and we call match().
set(kmimetypetest_SRCS kmimetypetest.cpp ../services/kmimemagicrule.cpp)
kde4_add_test(kdecore-kmimetypetest ${kmimetypetest_SRCS})
target_link_libraries(kdecore-kmimetypetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )

########### kmimeglobsfileparsertest ###############

# compile kmimeglobsfileparser.cpp into the test since it's not exported
set(kmimeglobsfileparsertest_SRCS kmimeglobsfileparsertest.cpp ../services/kmimeglobsfileparser.cpp)
kde4_add_test(kdecore-kmimeglobsfileparsertest ${kmimeglobsfileparsertest_SRCS})
target_link_libraries(kdecore-kmimeglobsfileparsertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )

########### kfiltertest ###############

# compile httpfilter.cpp into the test since it's not part of kdelibs
# (only par of kio_http and kmultipart)
set(kfiltertest_SRCS kfiltertest.cpp ../../kio/httpfilter/httpfilter.cc)
include_directories( ${CMAKE_SOURCE_DIR}/kio/httpfilter )
kde4_add_test(kdecore-kfiltertest ${kfiltertest_SRCS})
target_link_libraries(kdecore-kfiltertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${ZLIB_LIBRARIES})

########### module for klibloadertest4 ###############

set(klibloadertestmodule4_PART_SRCS klibloadertest4_module.cpp )

kde4_add_plugin(kdecore-klibloadertestmodule4 ${klibloadertestmodule4_PART_SRCS})

target_link_libraries(kdecore-klibloadertestmodule4  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

set_target_properties(kdecore-klibloadertestmodule4 PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)

########### kcompositejobtest ###############

set(kcompositejobtest_SRCS kcompositejobtest.cpp)
kde4_add_test(kdecore-kcompositejobtest ${kcompositejobtest_SRCS})
target_link_libraries(kdecore-kcompositejobtest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )
