
project(kconf_update)

include_directories( ${KDE4_KDECORE_INCLUDES} ${KDE4_KDEUI_INCLUDES} )


########### next target ###############

set(kconf_update_SRCS
    kconf_update.cpp
    kconfigutils.cpp
    )


kde4_add_executable(kconf_update NOGUI ${kconf_update_SRCS})

target_link_libraries(kconf_update  ${KDE4_KDECORE_LIBS} )

install(TARGETS kconf_update DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### install files ###############

add_subdirectory(tests)




