set(KDE_DEFAULT_HOME "@KDE_DEFAULT_HOME@")
set(KDE4_TARGET_PREFIX "@KDE4_TARGET_PREFIX@")

set(KDE4_INSTALL_DIR             "@CMAKE_INSTALL_PREFIX@")
set(KDE4_LIB_INSTALL_DIR         "@LIB_INSTALL_DIR@")
set(KDE4_IMPORTS_INSTALL_DIR     "@IMPORTS_INSTALL_DIR@")
set(KDE4_LIBEXEC_INSTALL_DIR     "@LIBEXEC_INSTALL_DIR@")
set(KDE4_INCLUDE_INSTALL_DIR     "@INCLUDE_INSTALL_DIR@")
set(KDE4_BIN_INSTALL_DIR         "@BIN_INSTALL_DIR@")
set(KDE4_SBIN_INSTALL_DIR        "@SBIN_INSTALL_DIR@")
set(KDE4_DATA_INSTALL_DIR        "@DATA_INSTALL_DIR@")
set(KDE4_CONFIG_INSTALL_DIR      "@CONFIG_INSTALL_DIR@")
set(KDE4_ICON_INSTALL_DIR        "@ICON_INSTALL_DIR@")
set(KDE4_KCFG_INSTALL_DIR        "@KCFG_INSTALL_DIR@")
set(KDE4_LOCALE_INSTALL_DIR      "@LOCALE_INSTALL_DIR@")
set(KDE4_MIME_INSTALL_DIR        "@MIME_INSTALL_DIR@")
set(KDE4_SOUND_INSTALL_DIR       "@SOUND_INSTALL_DIR@")
set(KDE4_TEMPLATES_INSTALL_DIR   "@TEMPLATES_INSTALL_DIR@")
set(KDE4_WALLPAPER_INSTALL_DIR   "@WALLPAPER_INSTALL_DIR@")
set(KDE4_AUTOSTART_INSTALL_DIR   "@AUTOSTART_INSTALL_DIR@")
set(KDE4_XDG_APPS_INSTALL_DIR    "@XDG_APPS_INSTALL_DIR@")
set(KDE4_XDG_DIRECTORY_INSTALL_DIR   "@XDG_DIRECTORY_INSTALL_DIR@")
set(KDE4_SYSCONF_INSTALL_DIR     "@SYSCONF_INSTALL_DIR@")
set(KDE4_MAN_INSTALL_DIR         "@MAN_INSTALL_DIR@")
set(KDE4_INFO_INSTALL_DIR        "@INFO_INSTALL_DIR@")
set(KDE4_DBUS_INTERFACES_DIR     "@DBUS_INTERFACES_INSTALL_DIR@")
set(KDE4_DBUS_SERVICES_DIR       "@DBUS_SERVICES_INSTALL_DIR@")
set(KDE4_SERVICES_INSTALL_DIR    "@SERVICES_INSTALL_DIR@")
set(KDE4_SERVICETYPES_INSTALL_DIR    "@SERVICETYPES_INSTALL_DIR@")

# those are for compatibility and will be gone in future release, they
# are set to ensure that installation directories override from vendors
# are respected trought all sub-packages
set(INSTALL_DIR             "@CMAKE_INSTALL_PREFIX@")
set(LIB_INSTALL_DIR         "@LIB_INSTALL_DIR@")
set(IMPORTS_INSTALL_DIR     "@IMPORTS_INSTALL_DIR@")
set(LIBEXEC_INSTALL_DIR     "@LIBEXEC_INSTALL_DIR@")
set(INCLUDE_INSTALL_DIR     "@INCLUDE_INSTALL_DIR@")
set(BIN_INSTALL_DIR         "@BIN_INSTALL_DIR@")
set(SBIN_INSTALL_DIR        "@SBIN_INSTALL_DIR@")
set(DATA_INSTALL_DIR        "@DATA_INSTALL_DIR@")
set(CONFIG_INSTALL_DIR      "@CONFIG_INSTALL_DIR@")
set(ICON_INSTALL_DIR        "@ICON_INSTALL_DIR@")
set(KCFG_INSTALL_DIR        "@KCFG_INSTALL_DIR@")
set(LOCALE_INSTALL_DIR      "@LOCALE_INSTALL_DIR@")
set(MIME_INSTALL_DIR        "@MIME_INSTALL_DIR@")
set(SOUND_INSTALL_DIR       "@SOUND_INSTALL_DIR@")
set(TEMPLATES_INSTALL_DIR   "@TEMPLATES_INSTALL_DIR@")
set(WALLPAPER_INSTALL_DIR   "@WALLPAPER_INSTALL_DIR@")
set(AUTOSTART_INSTALL_DIR   "@AUTOSTART_INSTALL_DIR@")
set(XDG_APPS_INSTALL_DIR    "@XDG_APPS_INSTALL_DIR@")
set(XDG_DIRECTORY_INSTALL_DIR   "@XDG_DIRECTORY_INSTALL_DIR@")
set(SYSCONF_INSTALL_DIR     "@SYSCONF_INSTALL_DIR@")
set(MAN_INSTALL_DIR         "@MAN_INSTALL_DIR@")
set(INFO_INSTALL_DIR        "@INFO_INSTALL_DIR@")
set(DBUS_INTERFACES_DIR     "@DBUS_INTERFACES_INSTALL_DIR@")
set(DBUS_SERVICES_DIR       "@DBUS_SERVICES_INSTALL_DIR@")
set(SERVICES_INSTALL_DIR    "@SERVICES_INSTALL_DIR@")
set(SERVICETYPES_INSTALL_DIR    "@SERVICETYPES_INSTALL_DIR@")

# This variable defines whether KPty::login/logout have been built with
# utempter support so that they don't require special user permissions
# in order to work properly. Used by kwrited.
set(KDE4_KPTY_BUILT_WITH_UTEMPTER @KDE4_KPTY_BUILT_WITH_UTEMPTER@)

