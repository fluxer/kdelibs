########################
# Helper backend probing

# not much of a choice really
set(KAUTH_BACKEND "DBus")

# Add the correct libraries/files depending on the backend
if(KAUTH_BACKEND STREQUAL "DBus")
    set(KAUTH_COMPILING_DBUS_HELPER_BACKEND TRUE)
    set(KDE4_AUTH_HELPER_BACKEND_NAME "DBUS")
    set(KDE4_AUTH_BACKEND_NAME "DBUS")
    set(KAUTH_BACKEND_SRCS
        auth/backends/dbus/DBusBackend.cpp
        auth/backends/dbus/DBusBackend.h
    )
    set(KAUTH_BACKEND_LIBS ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})

    qt4_add_dbus_adaptor(kauth_dbus_adaptor_SRCS
        auth/backends/dbus/org.kde.auth.xml
        auth/backends/dbus/DBusHelperProxy.h
        KAuth::DBusHelperProxy
    )

    set(KAUTH_HELPER_BACKEND_SRCS
        auth/backends/dbus/DBusHelperProxy.cpp
        ${kauth_dbus_adaptor_SRCS}
    )
    set(KAUTH_HELPER_BACKEND_LIBS kdecore)

    # Install some files as well
    install(
        FILES
        auth/backends/dbus/org.kde.auth.conf
        DESTINATION ${KDE4_SYSCONF_INSTALL_DIR}/dbus-1/system.d
    )

    install(
        FILES
        auth/backends/dbus/dbus_policy.stub
        auth/backends/dbus/dbus_service.stub
        DESTINATION ${KDE4_DATA_INSTALL_DIR}/kauth
        COMPONENT Devel
    )
endif()
