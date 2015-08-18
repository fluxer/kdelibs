project(kdeinit)

add_subdirectory( tests )

include_directories( ${KDE4_KPARTS_INCLUDES} )

########### Configuring (mainly for proctitle) ###########

include (CheckFunctionExists)
include (CheckIncludeFiles)
include (CheckVariableExists)
check_variable_exists(__progname HAVE___PROGNAME)
check_variable_exists(__progname_full HAVE___PROGNAME_FULL)
check_include_files(sys/pstat.h HAVE_SYS_PSTAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_function_exists(pstat HAVE_PSTAT)
check_function_exists(setproctitle HAVE_SETPROCTITLE)


# used by 4 executables in this file
if(HAVE_SOCKET_LIBRARY)
  set(KINIT_SOCKET_LIBRARY socket)
else(HAVE_SOCKET_LIBRARY)
  set(KINIT_SOCKET_LIBRARY)
endif(HAVE_SOCKET_LIBRARY)


########### kdeinit4 ###############

set(kdeinit_SRCS kinit.cpp proctitle.cpp klauncher_cmds.cpp )
set(kdeinit_LIBS "")

# on win32 kdeinit4 has to be a console application 
# to be able to catch stderr based --verbose output
kde4_add_executable(kdeinit4 NOGUI ${kdeinit_SRCS})

target_link_libraries(kdeinit4 ${KDE4_KDEUI_LIBS} ${kdeinit_LIBS} ${KINIT_SOCKET_LIBRARY} )
if (Q_WS_X11)
  target_link_libraries(kdeinit4 ${X11_X11_LIB})
endif(Q_WS_X11)

install(TARGETS kdeinit4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### kwrapper4 ###############

set(kwrapper_SRCS kwrapper.c )

kde4_add_executable(kwrapper4 NOGUI ${kwrapper_SRCS})

target_link_libraries(kwrapper4 ${KINIT_SOCKET_LIBRARY})

install(TARGETS kwrapper4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### kdeinit4_wrapper ###############

kde4_add_executable(kdeinit4_wrapper NOGUI wrapper.c)

target_link_libraries(kdeinit4_wrapper  ${KINIT_SOCKET_LIBRARY})

if (NOT Q_WS_X11 AND NOT Q_WS_QWS)
    add_definitions(-DNO_DISPLAY)
endif (NOT Q_WS_X11 AND NOT Q_WS_QWS)

install(TARGETS kdeinit4_wrapper ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### kshell4 ###############

kde4_add_executable(kshell4 NOGUI shell.c)

target_link_libraries(kshell4 ${KINIT_SOCKET_LIBRARY})

install(TARGETS kshell4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### kdeinit4_shutdown ###############

kde4_add_executable(kdeinit4_shutdown NOGUI wrapper.c)

target_link_libraries(kdeinit4_shutdown  ${KINIT_SOCKET_LIBRARY})

install(TARGETS kdeinit4_shutdown ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### lnusertemp ###############

set(lnusertemp_SRCS lnusertemp.c )

kde4_add_executable(lnusertemp NOGUI ${lnusertemp_SRCS})

target_link_libraries(lnusertemp kdefakes)

install(TARGETS lnusertemp DESTINATION ${LIBEXEC_INSTALL_DIR} )


########### kioslave ###############

set(kioslave_SRCS kioslave.cpp)


kde4_add_executable(kioslave NOGUI ${kioslave_SRCS})

target_link_libraries(kioslave  ${KDE4_KIO_LIBS} )

install(TARGETS kioslave DESTINATION ${LIBEXEC_INSTALL_DIR} )


########### klauncher ###############
set(klauncher_SRCS
   klauncher.cpp
   klauncher_main.cpp
   klauncher_adaptor.cpp
   autostart.cpp
   klauncher_cmds.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KLauncher.xml # just so that it gets generated
   )

# The adaptor is manually edited, generate the xml from it.
qt4_generate_dbus_interface(klauncher_adaptor.h org.kde.KLauncher.xml)


kde4_add_executable( klauncher NOGUI ${klauncher_SRCS})
set_target_properties(klauncher PROPERTIES DEFINE_SYMBOL MAKE_KLAUNCHER)

target_link_libraries(klauncher  ${KDE4_KIO_LIBS} ${X11_LIBRARIES})

install(TARGETS klauncher DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### install files ###############

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KLauncher.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

########### config-kdeinit.h ############
configure_file(config-kdeinit.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config-kdeinit.h )
