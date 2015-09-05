include_directories(${KDE4_KDEUI_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

#test5.cpp test5.h: $(srcdir)/test5.kcfg ../kconfig_compiler $(srcdir)/test5.kcfgc
#	../kconfig_compiler $(srcdir)/test5.kcfg $(srcdir)/test5.kcfgc

macro(ADD_KCONFIG_TEST _testName)
    set(_testSrc ${ARGN})
    kde4_add_kcfg_files(_testSrc ${_testName}.cpp)

    kde4_add_manual_test(${_testName} ${_testSrc})

    target_link_libraries(${_testName} ${KDE4_KDEUI_LIBS} ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})
endmacro()

########### next target ###############

ADD_KCONFIG_TEST(test1 test1main.cpp)

########### next target ###############

ADD_KCONFIG_TEST(test2 test2main.cpp )

########### next target ###############

ADD_KCONFIG_TEST(test3 test3main.cpp )

########### next target ###############

ADD_KCONFIG_TEST(test3a test3amain.cpp )

########### next target ###############

ADD_KCONFIG_TEST(test4 test4main.cpp )

########### next target ###############

ADD_KCONFIG_TEST(test5 test5main.cpp )

########### next target ###############

ADD_KCONFIG_TEST(test6 test6main.cpp )

########### next target ###############

ADD_KCONFIG_TEST(test7 test7main.cpp )

########### next target ###############

set(test8_SRCS test8main.cpp )

kde4_add_kcfg_files(test8a test8_SRCS)
kde4_add_kcfg_files(test8b test8_SRCS)

kde4_add_manual_test(test8 ${test8_SRCS})

target_link_libraries(test8 ${KDE4_KDEUI_LIBS} )


########### next target ###############

ADD_KCONFIG_TEST(test9 test9main.cpp )

########### next target ###############

ADD_KCONFIG_TEST(test10 test10main.cpp )

########### next target ###############

set(test11_SRCS test11main.cpp )

kde4_add_kcfg_files(test11 test11_SRCS)
kde4_add_kcfg_files(test11a test11_SRCS)

kde4_add_manual_test(test11 ${test11_SRCS})

target_link_libraries(test11  ${KDE4_KDEUI_LIBS} )


########### next target ###############

ADD_KCONFIG_TEST(test12 test12main.cpp )

########### next target ###############

ADD_KCONFIG_TEST(test_dpointer test_dpointer_main.cpp )

########### next target ###############

ADD_KCONFIG_TEST(test_signal test_signal_main.cpp )

########### next target ###############

ADD_KCONFIG_TEST(kconfigcompiler_test  kconfigcompiler_test.cpp )

