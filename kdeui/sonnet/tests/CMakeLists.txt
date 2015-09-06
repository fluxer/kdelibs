########### next target ###############

set(backgroundtest_SRCS backgroundtest.cpp)

kde4_add_manual_test(backgroundtest ${backgroundtest_SRCS})

target_link_libraries(backgroundtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(test_dialog_SRCS test_dialog.cpp)

kde4_add_manual_test(test_dialog ${test_dialog_SRCS})

target_link_libraries(test_dialog ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test_highlighter_SRCS test_highlighter.cpp)

kde4_add_manual_test(test_highlighter ${test_highlighter_SRCS})

target_link_libraries(test_highlighter ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test_configdialog_SRCS test_configdialog.cpp)

kde4_add_manual_test(test_configdialog ${test_configdialog_SRCS})

target_link_libraries(test_configdialog ${KDE4_KDEUI_LIBS} )

########### next target ###############

kde4_add_manual_test(test_dictionarycombobox test_dictionarycombobox.cpp)

target_link_libraries(test_dictionarycombobox ${KDE4_KDEUI_LIBS})
