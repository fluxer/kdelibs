project(kinit)

include_directories(
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1210)

########### kioslave ###############

add_executable(kioslave kioslave.cpp)

install(TARGETS kioslave DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR})

########### klauncher ###############

set(klauncher_SRCS
    klauncher.cpp
    klauncher_adaptor.cpp
    # just so that it gets generated
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KLauncher.xml
)

# Generate the xml from the adaptor.
qt4_generate_dbus_interface(klauncher_adaptor.h org.kde.KLauncher.xml)

add_executable(klauncher4 ${klauncher_SRCS})

target_link_libraries(klauncher4
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
)

kde4_add_dbus_service(org.kde.klauncher.service.in)

install(TARGETS klauncher4 DESTINATION ${KDE4_BIN_INSTALL_DIR})

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KLauncher.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)
