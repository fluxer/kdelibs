project(kdeinit)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

include_directories(${KDE4_KPARTS_INCLUDES})

########### Configuring (mainly for proctitle) ###########

include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckVariableExists)

check_variable_exists(__progname HAVE___PROGNAME)
check_variable_exists(__progname_full HAVE___PROGNAME_FULL)
check_include_files(sys/pstat.h HAVE_SYS_PSTAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_function_exists(pstat HAVE_PSTAT)
check_function_exists(setproctitle HAVE_SETPROCTITLE)

# used by 4 executables in this file
set(KINIT_SOCKET_LIBRARY)
if(HAVE_SOCKET_LIBRARY)
    set(KINIT_SOCKET_LIBRARY socket)
endif()

if (NOT Q_WS_X11 AND NOT Q_WS_QWS)
    add_definitions(-DNO_DISPLAY)
endif()

########### kdeinit4 ###############

set(kdeinit_SRCS
    kinit.cpp
    proctitle.cpp
    klauncher_cmds.cpp
)

add_executable(kdeinit4 ${kdeinit_SRCS})

target_link_libraries(kdeinit4 ${KDE4_KDEUI_LIBS} ${KINIT_SOCKET_LIBRARY})
if (Q_WS_X11)
    target_link_libraries(kdeinit4 ${X11_X11_LIB})
endif()

install(TARGETS kdeinit4 ${INSTALL_TARGETS_DEFAULT_ARGS})

########### kwrapper4 ###############

add_executable(kwrapper4 kwrapper.c)

target_link_libraries(kwrapper4 ${KINIT_SOCKET_LIBRARY})

install(TARGETS kwrapper4 ${INSTALL_TARGETS_DEFAULT_ARGS})

########### kdeinit4_wrapper ###############

add_executable(kdeinit4_wrapper wrapper.c)

target_link_libraries(kdeinit4_wrapper ${KINIT_SOCKET_LIBRARY})

install(TARGETS kdeinit4_wrapper ${INSTALL_TARGETS_DEFAULT_ARGS})

########### kdeinit4_shutdown ###############

add_executable(kdeinit4_shutdown wrapper.c)

target_link_libraries(kdeinit4_shutdown ${KINIT_SOCKET_LIBRARY})

install(TARGETS kdeinit4_shutdown ${INSTALL_TARGETS_DEFAULT_ARGS})

########### kshell4 ###############

add_executable(kshell4 shell.c)

target_link_libraries(kshell4 ${KINIT_SOCKET_LIBRARY})

install(TARGETS kshell4 ${INSTALL_TARGETS_DEFAULT_ARGS})

########### kioslave ###############

add_executable(kioslave kioslave.cpp)

target_link_libraries(kioslave ${QT_QTCORE_LIBRARY})

install(TARGETS kioslave DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR})


########### klauncher ###############
set(klauncher_SRCS
    klauncher.cpp
    klauncher_main.cpp
    klauncher_adaptor.cpp
    autostart.cpp
    klauncher_cmds.cpp
    # just so that it gets generated
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KLauncher.xml
)

# The adaptor is manually edited, generate the xml from it.
qt4_generate_dbus_interface(klauncher_adaptor.h org.kde.KLauncher.xml)

add_executable(klauncher ${klauncher_SRCS})

target_link_libraries(klauncher ${KDE4_KIO_LIBS} ${X11_LIBRARIES})

install(TARGETS klauncher DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR})

########### install files ###############

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KLauncher.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)

########### config-kdeinit.h ############

configure_file(
    config-kdeinit.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kdeinit.h
)
