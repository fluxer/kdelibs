# - Find the KDE4 include and library dirs, KDE preprocessors and define a some macros
#
# This module defines the following variables:
#
#  KDELIBS4_FOUND           - set to TRUE if everything required for building KDE software has been found
#
#  KDE4_DEFINITIONS         - compiler definitions required for compiling KDE software
#  KDE4_INCLUDES            - all include directories required for KDE, i.e.
#                             KDE4_INCLUDE_INSTALL_DIR, but also the Katie and X11 include directories
#  KDE4_LIB_INSTALL_DIR     - the directory where libraries from kdelibs are installed
#  KDE4_LIBEXEC_INSTALL_DIR - the directory where libexec executables from kdelibs are installed
#  KDE4_INCLUDE_INSTALL_DIR - the directory where headers from kdelibs are installed
#  KDE4_BIN_INSTALL_DIR     - the directory where executables from kdelibs are installed
#  KDE4_SBIN_INSTALL_DIR    - the directory where system executables from kdelibs are installed
#  KDE4_DATA_INSTALL_DIR    - the parent directory where kdelibs applications install their data
#  KDE4_CONFIG_INSTALL_DIR  - the directory where config files from kdelibs are installed
#  KDE4_ICON_INSTALL_DIR    - the directory where icons from kdelibs are
#  KDE4_IMPORTS_INSTALL_DIR - the directory where imports from kdelibs are
#  KDE4_KCFG_INSTALL_DIR    - the directory where kconfig files from kdelibs are installed
#  KDE4_LOCALE_INSTALL_DIR  - the directory where translations from kdelibs are installed
#  KDE4_MIME_INSTALL_DIR    - the directory where mimetype desktop files from kdelibs are installed
#  KDE4_SOUND_INSTALL_DIR   - the directory where sound files from kdelibs are installed
#  KDE4_TEMPLATES_INSTALL_DIR     - the directory where templates (Create new file...) from kdelibs are installed
#  KDE4_WALLPAPER_INSTALL_DIR     - the directory where wallpapers from kdelibs are installed
#  KDE4_AUTOSTART_INSTALL_DIR     - the directory where autostart from kdelibs are installed
#  KDE4_XDG_APPS_INSTALL_DIR      - the XDG apps dir from kdelibs
#  KDE4_XDG_DIRECTORY_INSTALL_DIR - the XDG directory from kdelibs
#  KDE4_XDG_MIME_INSTALL_DIR      - the XDG mimetypes install dir from kdelibs
#  KDE4_SYSCONF_INSTALL_DIR       - the directory where sysconfig files from kdelibs are installed
#  KDE4_SERVICES_INSTALL_DIR      - the directory where service (desktop, protocol, ...) files from kdelibs are installed
#  KDE4_SERVICETYPES_INSTALL_DIR  - the directory where servicestypes desktop files from kdelibs are installed
#  KDE4_DBUS_INTERFACES_DIR       - the directory where dbus interfaces from kdelibs are installed
#  KDE4_DBUS_SERVICES_INSTALL_DIR        - the directory where dbus service files from kdelibs are installed
#  KDE4_DBUS_SYSTEM_SERVICES_INSTALL_DIR - the directory where dbus system services from kdelibs are installed
#
# The following variables are defined for the various tools required to
# compile KDE software:
#
#  KDE4_KCFGC_EXECUTABLE    - the kconfig_compiler executable
#  KDE4_MAKEKDEWIDGETS_EXECUTABLE - the makekdewidgets executable
#
#  This module allows to depend on a particular minimum version of kdelibs.
#  To acomplish that one should use the appropriate cmake syntax for
#  find_package. For example to depend on kdelibs >= 4.23.0 one should use
#
#  find_package(KDELibs4 4.23.0 REQUIRED)
#
# Copyright (c) 2006-2009, Alexander Neundorf <neundorf@kde.org>
# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Only do something if it hasn't been found yet
if(NOT KDELIBS4_FOUND)
    set(KDELIBS4_FOUND TRUE)

    set(KDE4_DEFINITIONS "@KDE4_DEFINITIONS@")
    set(KDE_DEFAULT_HOME "@KDE_DEFAULT_HOME@")
    set(KDE4_TARGET_PREFIX "@KDE4_TARGET_PREFIX@")
    set(KDE4_ENABLE_EXCEPTIONS "@KDE4_ENABLE_EXCEPTIONS@")

    set(KDE4_INSTALL_DIR             "@CMAKE_INSTALL_PREFIX@")
    set(KDE4_EXEC_INSTALL_PREFIX  "@KDE4_EXEC_INSTALL_PREFIX@")
    set(KDE4_SHARE_INSTALL_PREFIX "@KDE4_SHARE_INSTALL_PREFIX@")
    set(KDE4_BIN_INSTALL_DIR      "@KDE4_BIN_INSTALL_DIR@")
    set(KDE4_SBIN_INSTALL_DIR     "@KDE4_SBIN_INSTALL_DIR@")
    set(KDE4_LIB_INSTALL_DIR      "@KDE4_LIB_INSTALL_DIR@")
    set(KDE4_LIBEXEC_INSTALL_DIR  "@KDE4_LIBEXEC_INSTALL_DIR@")
    set(KDE4_INCLUDE_INSTALL_DIR  "@KDE4_INCLUDE_INSTALL_DIR@")
    set(KDE4_PLUGIN_INSTALL_DIR       "@KDE4_PLUGIN_INSTALL_DIR@")
    set(KDE4_IMPORTS_INSTALL_DIR      "@KDE4_IMPORTS_INSTALL_DIR@")
    set(KDE4_CONFIG_INSTALL_DIR       "@KDE4_CONFIG_INSTALL_DIR@")
    set(KDE4_DATA_INSTALL_DIR         "@KDE4_DATA_INSTALL_DIR@")
    set(KDE4_ICON_INSTALL_DIR         "@KDE4_ICON_INSTALL_DIR@")
    set(KDE4_KCFG_INSTALL_DIR         "@KDE4_KCFG_INSTALL_DIR@")
    set(KDE4_LOCALE_INSTALL_DIR       "@KDE4_LOCALE_INSTALL_DIR@")
    set(KDE4_SERVICES_INSTALL_DIR     "@KDE4_SERVICES_INSTALL_DIR@")
    set(KDE4_SERVICETYPES_INSTALL_DIR "@KDE4_SERVICETYPES_INSTALL_DIR@")
    set(KDE4_SOUND_INSTALL_DIR        "@KDE4_SOUND_INSTALL_DIR@")
    set(KDE4_TEMPLATES_INSTALL_DIR    "@KDE4_TEMPLATES_INSTALL_DIR@")
    set(KDE4_WALLPAPER_INSTALL_DIR    "@KDE4_WALLPAPER_INSTALL_DIR@")
    set(KDE4_AUTOSTART_INSTALL_DIR    "@KDE4_AUTOSTART_INSTALL_DIR@")
    set(KDE4_XDG_APPS_INSTALL_DIR     "@KDE4_XDG_APPS_INSTALL_DIR@")
    set(KDE4_XDG_DIRECTORY_INSTALL_DIR "@KDE4_XDG_DIRECTORY_INSTALL_DIR@")
    set(KDE4_XDG_MIME_INSTALL_DIR     "@KDE4_XDG_MIME_INSTALL_DIR@")
    set(KDE4_SYSCONF_INSTALL_DIR      "@KDE4_SYSCONF_INSTALL_DIR@")
    set(KDE4_DBUS_INTERFACES_INSTALL_DIR "@KDE4_DBUS_INTERFACES_INSTALL_DIR@")
    set(KDE4_DBUS_SERVICES_INSTALL_DIR "@KDE4_DBUS_SERVICES_INSTALL_DIR@")
    set(KDE4_DBUS_SYSTEM_SERVICES_INSTALL_DIR "@KDE4_DBUS_SYSTEM_SERVICES_INSTALL_DIR@")

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(KDE4_DEFINITIONS "${KDE4_DEFINITIONS} -DNDEBUG")
    endif()

    find_package(Katie QUIET REQUIRED 4.13.0)
    find_package(X11 QUIET REQUIRED)

    # add the found Katie and KDE include directories to the current include path
    # the ${KDE4_INCLUDE_INSTALL_DIR}/KDE directory is for forwarding includes, e.g.
    # #include <KMainWindow>
    set(KDE4_INCLUDES
        ${KDE4_INCLUDE_INSTALL_DIR}
        ${KDE4_INCLUDE_INSTALL_DIR}/KDE
        ${QT_INCLUDES}
        ${X11_INCLUDE_DIR}
    )

    # add some more default search paths
    set(CMAKE_SYSTEM_INCLUDE_PATH
        ${CMAKE_SYSTEM_INCLUDE_PATH}
        "${KDE4_INCLUDE_INSTALL_DIR}"
    )

    set(CMAKE_SYSTEM_PROGRAM_PATH
        ${CMAKE_SYSTEM_PROGRAM_PATH}
        "${KDE4_BIN_INSTALL_DIR}"
    )

    set(CMAKE_SYSTEM_LIBRARY_PATH
        ${CMAKE_SYSTEM_LIBRARY_PATH}
        "${KDE4_LIB_INSTALL_DIR}"
    )

    set(kde_cmake_module_dir "${KDE4_DATA_INSTALL_DIR}/cmake/modules")
    # get the directory of the current file, used later on in the file
    get_filename_component(kdelibs4_config_dir ${CMAKE_CURRENT_LIST_FILE} PATH)

    # allow searching cmake modules in the kde install locations
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${kde_cmake_module_dir}")

    # KDELibs4ConfigVersion.cmake contains KDE version
    include(${kdelibs4_config_dir}/KDELibs4ConfigVersion.cmake)

    # KDE4Defaults.cmake contains KDE defaults
    include(${kdelibs4_config_dir}/KDE4Defaults.cmake)

    # Now include the file with the imported tools (executable targets).
    # This export-file is generated and installed by the toplevel CMakeLists.txt of kdelibs.
    # Having the libs and tools in two separate files should help with cross compiling.
    include(${kdelibs4_config_dir}/KDELibs4ToolsTargets.cmake)

    set(KDE4_KCFGC_EXECUTABLE             ${KDE4_TARGET_PREFIX}kconfig_compiler)
    set(KDE4_MAKEKDEWIDGETS_EXECUTABLE    ${KDE4_TARGET_PREFIX}makekdewidgets)

    # This file contains the exported library target from kdelibs (new with cmake 2.6.x), e.g.
    # the library target "kdeui" is exported as "KDE4::kdeui". The "KDE4::" is used as
    # "namespace" to separate the imported targets from "normal" targets, it is stored in
    # KDE4_TARGET_PREFIX.
    # This export-file is generated and installed by the toplevel CMakeLists.txt of kdelibs.
    # Include it to "import" the libraries from kdelibs into the current projects as targets.
    include(${kdelibs4_config_dir}/KDELibs4LibraryTargets.cmake)

    # KDE4Macros.cmake contains all the KDE specific macros
    include(${kdelibs4_config_dir}/KDE4Macros.cmake)

    configure_file(
        "${kde_cmake_module_dir}/kde4_exec.sh.in"
        "${CMAKE_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/kde4_exec.sh"
        @ONLY
    )
    file(
        COPY "${CMAKE_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/kde4_exec.sh"
        DESTINATION "${CMAKE_BINARY_DIR}"
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ
    )

    if(NOT KDELIBS4_FIND_QUIETLY)
        message(STATUS "Found KDE version: ${KDE_VERSION}")
    endif()
endif()
