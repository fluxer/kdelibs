check_include_files("sys/prctl.h" HAVE_SYS_PRCTL_H)
check_symbol_exists(PR_SET_DUMPABLE "sys/prctl.h" HAVE_PR_SET_DUMPABLE)
add_feature_info("prctl-dumpable" HAVE_PR_SET_DUMPABLE "Used to disallow ptracing")

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include_directories(
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
)

#  set(kt4_SRC app.cpp mainwindow.cpp core.cpp view.cpp viewmodel.cpp main.cpp)
set(KDESUDO_SRC
    main.cpp
    kdesudo.cpp
    kcookie.cpp
)

add_executable(kdesudo ${KDESUDO_SRC})

target_link_libraries(kdesudo ${KDE4_KDEUI_LIBS})

install(
    TARGETS kdesudo
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)
