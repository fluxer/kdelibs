########### next target ###############

set(backgroundtest_SRCS backgroundtest.cpp)

add_executable(backgroundtest ${backgroundtest_SRCS})

target_link_libraries(backgroundtest ${KDE4_KDEUI_LIBS})


########### next target ###############

set(test_dialog_SRCS test_dialog.cpp)

add_executable(test_dialog ${test_dialog_SRCS})

target_link_libraries(test_dialog ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test_highlighter_SRCS test_highlighter.cpp)

add_executable(test_highlighter ${test_highlighter_SRCS})

target_link_libraries(test_highlighter ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(test_configdialog_SRCS test_configdialog.cpp)

add_executable(test_configdialog ${test_configdialog_SRCS})

target_link_libraries(test_configdialog ${KDE4_KDEUI_LIBS} )

########### next target ###############

add_executable(test_dictionarycombobox test_dictionarycombobox.cpp)

target_link_libraries(test_dictionarycombobox ${KDE4_KDEUI_LIBS})
