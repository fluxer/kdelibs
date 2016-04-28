# - Find the KDE4 include and library dirs, KDE preprocessors and define a some macros
#
# This module defines the following variables:
#
#  KDE4_FOUND               - set to TRUE if everything required for building KDE software has been found
#
#  KDE4_DEFINITIONS         - compiler definitions required for compiling KDE software
#  KDE4_INCLUDE_DIR         - the KDE 4 include directory
#  KDE4_INCLUDES            - all include directories required for KDE, i.e.
#                             KDE4_INCLUDE_DIR, but also the Qt4 include directories
#                             and other platform specific include directories
#  KDE4_LIB_DIR             - the directory where the KDE libraries are installed,
#                             intended to be used with LINK_DIRECTORIES(). In general, this is not necessary.
#  KDE4_LIBEXEC_INSTALL_DIR - the directory where libexec executables from kdelibs are installed
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
#  KDE4_SYSCONF_INSTALL_DIR       - the directory where sysconfig files from kdelibs are installed
#  KDE4_MAN_INSTALL_DIR           - the directory where man pages from kdelibs are installed
#  KDE4_INFO_INSTALL_DIR          - the directory where info files from kdelibs are installed
#  KDE4_DBUS_INTERFACES_DIR       - the directory where dbus interfaces from kdelibs are installed
#  KDE4_DBUS_SERVICES_DIR         - the directory where dbus service files from kdelibs are installed
#
# The following variables are defined for the various tools required to
# compile KDE software:
#
#  KDE4_KCFGC_EXECUTABLE    - the kconfig_compiler executable
#  KDE4_MAKEKDEWIDGETS_EXECUTABLE - the makekdewidgets executable
#
# The following variables point to the location of the KDE libraries,
# but shouldn't be used directly:
#
#  KDE4_KDECORE_LIBRARY     - the kdecore library
#  KDE4_KDEUI_LIBRARY       - the kdeui library
#  KDE4_KIO_LIBRARY         - the kio library
#  KDE4_KPARTS_LIBRARY      - the kparts library
#  KDE4_KEMOTICONS_LIBRARY  - the kemoticons library
#  KDE4_KIDLETIME_LIBRARY   - the kidletime library
#  KDE4_KCMUTILS_LIBRARY    - the kcmutils library
#  KDE4_KPRINTUTILS_LIBRARY - the kprintutils library
#  KDE4_KFILE_LIBRARY       - the kfile library
#  KDE4_KDNSSD_LIBRARY      - the kdnssd library
#  KDE4_THREADWEAVER_LIBRARY- the threadweaver library
#  KDE4_SOLID_LIBRARY       - the solid library
#  KDE4_KNOTIFYCONFIG_LIBRARY- the knotifyconfig library
#  KDE4_KROSSCORE_LIBRARY   - the krosscore library
#  KDE4_KTEXTEDITOR_LIBRARY - the ktexteditor library
#  KDE4_PLASMA_LIBRARY      - the plasma library
#  KDE4_KUNITCONVERSION_LIBRARY - the kunitconversion library
#  KDE4_KDEWEBKIT_LIBRARY   - the kdewebkit library
#  KDE4_KCDDB_LIBRARY       - the kcddb library
#  KDE4_KDCRAW_LIBRARY       - the kdcraw library
#  KDE4_KEXIV2_LIBRARY       - the kexiv2 library
#
# Compared to the variables above, the following variables
# also contain all of the depending libraries, so the variables below
# should be used instead of the ones above:
#
#  KDE4_KDECORE_LIBS          - the kdecore library and all depending libraries
#  KDE4_KDEUI_LIBS            - the kdeui library and all depending libraries
#  KDE4_KIO_LIBS              - the kio library and all depending libraries
#  KDE4_KPARTS_LIBS           - the kparts library and all depending libraries
#  KDE4_KEMOTICONS_LIBS       - the kemoticons library and all depending libraries
#  KDE4_KIDLETIME_LIBS        - the kidletime library and all depending libraries
#  KDE4_KCMUTILS_LIBS         - the kcmutils library and all depending libraries
#  KDE4_KPRINTUTILS_LIBS      - the kprintutils library and all depending libraries
#  KDE4_KFILE_LIBS            - the kfile library and all depending libraries
#  KDE4_KDNSSD_LIBS           - the kdnssd library and all depending libraries
#  KDE4_KDESU_LIBS            - the kdesu library and all depending libraries
#  KDE4_KPTY_LIBS             - the kpty library and all depending libraries
#  KDE4_THREADWEAVER_LIBS     - the threadweaver library and all depending libraries
#  KDE4_SOLID_LIBS            - the solid library and all depending libraries
#  KDE4_KNOTIFYCONFIG_LIBS    - the knotify config library and all depending libraries
#  KDE4_KROSSCORE_LIBS        - the kross core library and all depending libraries
#  KDE4_KROSSUI_LIBS          - the kross ui library which includes core and all depending libraries
#  KDE4_KTEXTEDITOR_LIBS      - the ktexteditor library and all depending libraries
#  KDE4_PLASMA_LIBS           - the plasma library and all depending librairies
#  KDE4_KUNITCONVERSION_LIBS  - the kunitconversion library and all depending libraries
#  KDE4_KDEWEBKIT_LIBS        - the kdewebkit library and all depending libraries
#  KDE4_KCDDB_LIBS            - the kcddb library and all depending libraries
#  KDE4_KDCRAW_LIBS           - the kdcraw library and all depending libraries
#  KDE4_KEXIV2_LIBS           - the kexiv2 library and all depending libraries
#
# This module defines also a bunch of variables used as locations for install directories
# for files of the package which is using this module. These variables don't say
# anything about the location of the installed KDE.
# They can be relative (to CMAKE_INSTALL_PREFIX) or absolute.
#
#  BIN_INSTALL_DIR          - the directory where executables will be installed (default is prefix/bin)
#  SBIN_INSTALL_DIR         - the directory where system executables will be installed (default is prefix/sbin)
#  LIB_INSTALL_DIR          - the directory where libraries will be installed (default is prefix/lib)
#  CONFIG_INSTALL_DIR       - the directory where config files will be installed
#  DATA_INSTALL_DIR         - the parent directory where applications can install their data
#  ICON_INSTALL_DIR         - the directory where the icons will be installed (default prefix/share/icons/)
#  INFO_INSTALL_DIR         - the directory where info files will be installed (default prefix/info)
#  KCFG_INSTALL_DIR         - the directory where kconfig files will be installed
#  LOCALE_INSTALL_DIR       - the directory where translations will be installed
#  MAN_INSTALL_DIR          - the directory where man pages will be installed (default prefix/man/)
#  MIME_INSTALL_DIR         - the directory where mimetype desktop files will be installed
#  PLUGIN_INSTALL_DIR       - the subdirectory relative to the install prefix where plugins will be installed (default is ${KDE4_LIB_INSTALL_DIR})
#  IMPORTS_INSTALL_DIR      - the subdirectory relative to the install prefix where imports will be installed
#  SERVICES_INSTALL_DIR     - the directory where service (desktop, protocol, ...) files will be installed
#  SERVICETYPES_INSTALL_DIR - the directory where servicestypes desktop files will be installed
#  SOUND_INSTALL_DIR        - the directory where sound files will be installed
#  TEMPLATES_INSTALL_DIR    - the directory where templates (Create new file...) will be installed
#  WALLPAPER_INSTALL_DIR    - the directory where wallpapers will be installed
#  AUTOSTART_INSTALL_DIR    - the directory where autostart files will be installed
#  SYSCONF_INSTALL_DIR      - the directory where sysconfig files will be installed (default /usr/etc)
#  XDG_APPS_INSTALL_DIR     - the XDG apps dir
#  XDG_DIRECTORY_INSTALL_DIR- the XDG directory
#  XDG_MIME_INSTALL_DIR     - the XDG mimetypes install dir
#  DBUS_INTERFACES_INSTALL_DIR - the directory where dbus interfaces will be installed (default is prefix/share/dbus-1/interfaces)
#  DBUS_SERVICES_INSTALL_DIR        - the directory where dbus services will be installed (default is prefix/share/dbus-1/services )
#  DBUS_SYSTEM_SERVICES_INSTALL_DIR        - the directory where dbus system services will be installed (default is prefix/share/dbus-1/system-services )
#
# The variable INSTALL_TARGETS_DEFAULT_ARGS can be used when installing libraries
# or executables into the default locations.
# The INSTALL_TARGETS_DEFAULT_ARGS variable should be used when libraries are installed.
# It should also be used when installing applications.
# The variable MUST NOT be used for installing plugins.
# It also MUST NOT be used for executables which are intended to go into sbin/ or libexec/.
#
# Usage is like this:
#    install(TARGETS kdecore kdeui ${INSTALL_TARGETS_DEFAULT_ARGS} )
#
# This will install libraries correctly under UNIX, OSX and Windows (i.e. dll's go
# into bin/.
#
#
# The following user adjustable options are provided:
#
#  KDE4_ADD_KCFG_FILES (SRCS_VAR [GENERATE_MOC] [USE_RELATIVE_PATH] file1.kcfgc ... fileN.kcfgc)
#    Use this to add KDE config compiler files to your application/library.
#    Use optional GENERATE_MOC to generate moc if you use signals in your kcfg files.
#    Use optional USE_RELATIVE_PATH to generate the classes in the build following the given
#    relative path to the file.
#
#  KDE4_ADD_WIDGET (SRCS_VAR file1.widgets ... fileN.widgets)
#    Use this to add widget description files for the makekdewidgets code generator
#    for Qt Designer plugins.
#
#  KDE4_ADD_PLUGIN ( name [WITH_PREFIX] file1 ... fileN )
#    Create a KDE plugin (KPart, kioslave, etc.) from the given source files.
#    If WITH_PREFIX is given, the resulting plugin will have the prefix "lib", otherwise it won't.
#
#  KDE4_ADD_TEST (testname file1 ... fileN)
#    add a unit test, which is executed when running make test. The targets
#    are build and executed only if the ENABLE_TESTING option is enabled.
#    KDESRCDIR is set to the source directory of the test, this can be used
#    with KGlobal::dirs()->addResourceDir( "data", KDESRCDIR )
#
#  KDE4_ADD_MANUAL_TEST (testname file1 ... fileN)
#    same as KDE_ADD_TEST() except that the test is not run on `make test`
#
#  KDE4_INSTALL_ICONS ( path theme)
#    Installs all png and svgz files in the current directory to the icon
#    directory given in path, in the subdirectory for the given icon theme.
#
#  KDE4_CREATE_MANPAGE ( docbookfile section )
#   Create the manpage for the specified section from the docbookfile
#   The resulting manpage will be installed to <installdest> when using
#   INSTALL_DESTINATION <installdest>, or to <installdest>/<subdir> if
#   SUBDIR <subdir> is specified.
#
#  KDE4_INSTALL_AUTH_HELPER_FILES ( HELPER_TARGET HELPER_ID HELPER_USER )
#   This macro adds the needed files for an helper executable meant to be used by applications using KAuth.
#   It accepts the helper target, the helper ID (the DBUS name) and the user under which the helper will run on.
#   This macro takes care of generate the needed files, and install them in the right location. This boils down
#   to a DBus policy to let the helper register on the system bus, and a service file for letting the helper
#   being automatically activated by the system bus.
#   *WARNING* You have to install the helper in ${LIBEXEC_INSTALL_DIR} to make sure everything will work.
#
#
#
#  A note on the possible values for CMAKE_BUILD_TYPE and how KDE handles
#  the flags for those buildtypes. FindKDE4Internal supports the values
#  Release, RelWithDebInfo and Debug:
#
#  Release
#          optimised for speed, qDebug/kDebug turned off, no debug symbols, no asserts
#  RelWithDebInfo (Release with debug info)
#          similar to Release, optimised for speed, but with debugging symbols on (-g)
#  Debug
#          optimised but debuggable, debugging on (-g)
#          (-fno-reorder-blocks -fno-schedule-insns -fno-inline)
#  MinSizeRel:
#          optimization for smallest size, no debugging information
#
#
#  The default buildtype is RelWithDebInfo.
#  It is expected that the "Debug" build type be still debuggable with gdb
#  without going all over the place, but still produce better performance.
#  It's also important to note that gcc cannot detect all warning conditions
#  unless the optimiser is active.
#
#
#  This module allows to depend on a particular minimum version of kdelibs.
#  To acomplish that one should use the appropriate cmake syntax for
#  find_package. For example to depend on kdelibs >= 4.1.0 one should use
#
#  find_package(KDE4 4.18.0 REQUIRED)
#
#  In earlier versions of KDE you could use the variable KDE_MIN_VERSION to
#  have such a dependency. This variable is deprecated with KDE 4.2.0, but
#  will still work to make the module backwards-compatible.

#  _KDE4_PLATFORM_INCLUDE_DIRS is used only internally
#  _KDE4_PLATFORM_DEFINITIONS is used only internally

# Copyright (c) 2006-2009, Alexander Neundorf <neundorf@kde.org>
# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


# this is required by cmake >=2.6
cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

# CMP0000: don't require cmake_minimum_version() directly in the top level
# CMakeLists.txt, FindKDE4Internal.cmake is good enough
cmake_policy(SET CMP0000 OLD)
# CMP0003: add the link paths to the link command as with cmake 2.4
cmake_policy(SET CMP0003 OLD)
# TODO: get rid of this and adjust to new behaviour
# CMP0005: keep escaping behaviour for definitions added via add_definitions()
cmake_policy(SET CMP0005 OLD)

# Only do something if it hasn't been found yet
if(NOT KDE4_FOUND)

# get the directory of the current file, used later on in the file
get_filename_component(kde_cmake_module_dir ${CMAKE_CURRENT_LIST_FILE} PATH)

# Store CMAKE_MODULE_PATH and then append the current dir to it, so we are sure
# we get the FindQt4.cmake located next to us and not a different one.
# The original CMAKE_MODULE_PATH is restored later on.
set(_kde_cmake_module_path_back ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${kde_cmake_module_dir} ${CMAKE_MODULE_PATH} )

# if the minimum Qt requirement is changed, change all occurrence in the
# following lines
if(NOT QT_MIN_VERSION OR QT_MIN_VERSION VERSION_LESS "4.8.2")
    set(QT_MIN_VERSION "4.8.2")
endif()

# Tell FindQt4.cmake to point the QT_QTFOO_LIBRARY targets at the imported targets
# for the Qt libraries, so we get full handling of release and debug versions of the
# Qt libs and are flexible regarding the install location of Qt under Windows:
set(QT_USE_IMPORTED_TARGETS TRUE)

# We may only search for other packages with "REQUIRED" if we are required ourselves.
# This file can be processed either (usually) included in FindKDE4.cmake or
# (when building kdelibs) directly via FIND_PACKAGE(KDE4Internal), that's why
# we have to check for both KDE4_FIND_REQUIRED and KDE4Internal_FIND_REQUIRED.
if(KDE4_FIND_REQUIRED OR KDE4Internal_FIND_REQUIRED)
    set(_REQ_STRING_KDE4 REQUIRED)
endif()

#this line includes FindQt4.cmake, which searches the Qt library and headers
# TODO: we should check here that all necessary modules of Qt have been found, e.g. QtDBus
option(WITH_KATIE "Build against Katie instead of Qt4" ON)

# TODO: once Katie goes stable make it required from const in KDEConfig if
# kdelibs is build against it, this file may go away due to order issues
# and be merged into KDEConfig.
if(WITH_KATIE)
    find_package(Katie)
endif()
if(NOT KATIE_FOUND)
    # avoid the need to check WITH_KATIE in addition to KATIE_FOUND
    set(KATIE_FOUND FALSE)

    find_package(Qt4 ${_REQ_STRING_KDE4})
endif()

# restore the original CMAKE_MODULE_PATH
set(CMAKE_MODULE_PATH ${_kde_cmake_module_path_back})

# Check that we really found everything.
# If KDE4 was searched with REQUIRED, we error out with FATAL_ERROR if something
# wasn't found already above in the other FIND_PACKAGE() calls. If KDE4 was
# searched without REQUIRED and something in the FIND_PACKAGE() calls above
# wasn't found,then we get here and must check that everything has actually
# been found. If something is missing, we must not fail with FATAL_ERROR, but only not set KDE4_FOUND.
if(NOT QT4_FOUND)
    message(STATUS "KDE4 not found, because Qt4 was not found")
    return()
endif(NOT QT4_FOUND)

# now we are sure we have everything we need
include(MacroLibrary)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)

# are we trying to compile kdelibs? then enter bootstrap mode
# kdelibs_SOURCE_DIR comes from "project(kdelibs)" in kdelibs/CMakeLists.txt

if(kdelibs_SOURCE_DIR)
    set(_kdeBootStrapping TRUE)
    message(STATUS "Building kdelibs...")
else(kdelibs_SOURCE_DIR)
    set(_kdeBootStrapping FALSE)
endif(kdelibs_SOURCE_DIR)

# Used in configure_file() and install(EXPORT)
set(KDE4_TARGET_PREFIX KDE4::)

#######################  #now try to find some kde stuff  ################################

if (_kdeBootStrapping)
    set(KDE4_INCLUDE_DIR ${kdelibs_SOURCE_DIR})

    set(EXECUTABLE_OUTPUT_PATH ${kdelibs_BINARY_DIR}/bin )

    set(LIBRARY_OUTPUT_PATH               ${CMAKE_BINARY_DIR}/lib )
    set(KDE4_KCFGC_EXECUTABLE             kconfig_compiler${CMAKE_EXECUTABLE_SUFFIX} )
    set(KDE4_MAKEKDEWIDGETS_EXECUTABLE    makekdewidgets${CMAKE_EXECUTABLE_SUFFIX} )

    set(KDE4_LIB_DIR ${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR})

    set(KDE4_INSTALLED_VERSION_OK TRUE)
else(_kdeBootStrapping)
    set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib )

    # These files contain information about the installed kdelibs
    include(${kde_cmake_module_dir}/KDE4Config.cmake)
    include(${kde_cmake_module_dir}/KDE4Version.cmake)

    # Check the version of KDE. It must be at least KDE_MIN_VERSION as set by the user.
    # KDE_VERSION is set in KDE4Version.cmake since KDE 4.17.x.
    set(KDE4_INSTALLED_VERSION_OK FALSE)
    if(NOT "${KDE_VERSION}" VERSION_LESS "${KDE_MIN_VERSION}")
        set(KDE4_INSTALLED_VERSION_OK TRUE)
    endif()


    # KDE4_LIB_INSTALL_DIR and KDE4_INCLUDE_INSTALL_DIR are set in KDE4Config.cmake,
    # use them to set the KDE4_LIB_DIR and KDE4_INCLUDE_DIR "public interface" variables
    set(KDE4_LIB_DIR ${KDE4_LIB_INSTALL_DIR})
    set(KDE4_INCLUDE_DIR ${KDE4_INCLUDE_INSTALL_DIR})

    # Now include the file with the imported tools (executable targets).
    # This export-file is generated and installed by the toplevel CMakeLists.txt of kdelibs.
    # Having the libs and tools in two separate files should help with cross compiling.
    include(${kde_cmake_module_dir}/KDELibs4ToolsTargets.cmake)

    set(KDE4_KCFGC_EXECUTABLE             ${KDE4_TARGET_PREFIX}kconfig_compiler)
    set(KDE4_MAKEKDEWIDGETS_EXECUTABLE    ${KDE4_TARGET_PREFIX}makekdewidgets)

    # allow searching cmake modules in all given kde install locations (KDEDIRS based)
    execute_process(
        COMMAND "${KDE4_KDECONFIG_EXECUTABLE}" --path data
        OUTPUT_VARIABLE _data_DIR
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    file(TO_CMAKE_PATH "${_data_DIR}" _data_DIR)
    foreach(dir ${_data_DIR})
        set(apath "${dir}/cmake/modules")
        if(EXISTS "${apath}")
            string(TOLOWER "${apath}" _apath)
            # ignore already added pathes, case insensitive
            foreach(adir ${CMAKE_MODULE_PATH})
                string(TOLOWER "${adir}" _adir)
                if(NOT "${_adir}" STREQUAL "${_apath}")
                    message(STATUS "Adding ${apath} to CMAKE_MODULE_PATH")
                    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${apath}")
                endif()
            endforeach()
        endif()
    endforeach(dir)


    # This file contains the exported library target from kdelibs (new with cmake 2.6.x), e.g.
    # the library target "kdeui" is exported as "KDE4::kdeui". The "KDE4::" is used as
    # "namespace" to separate the imported targets from "normal" targets, it is stored in
    # KDE4_TARGET_PREFIX, which is set in KDELibsDependencies.cmake .
    # This export-file is generated and installed by the toplevel CMakeLists.txt of kdelibs.
    # Include it to "import" the libraries from kdelibs into the current projects as targets.
    # This makes setting the _LIBRARY and _LIBS variables actually a bit superfluos, since e.g.
    # the kdeui library could now also be used just as "KDE4::kdeui" and still have all their
    # dependent libraries handled correctly. But to keep compatibility and not to change
    # behaviour we set all these variables anyway as seen below. Alex
    include(${kde_cmake_module_dir}/KDELibs4LibraryTargets.cmake)
endif(_kdeBootStrapping)

# Set the various KDE4_FOO_LIBRARY/LIBS variables.
# In bootstrapping mode KDE4_TARGET_PREFIX is empty, so e.g. KDE4_KDECORE_LIBRARY
# will be simply set to "kdecore".
set(_kde_libraries
    kcddb
    kcmutils
    kdcraw
    kdeclarative
    kdecore
    kdesu
    kdeui
    kdewebkit
    kdnssd
    kemoticons
    kexiv2
    kfile
    kidletime
    kio
    knotifyconfig
    kparts
    kprintutils
    kpty
    krosscore
    krossui
    ktexteditor
    kunitconversion
    plasma
    solid
    threadweaver
)
foreach(_lib ${_kde_libraries})
    string(TOUPPER ${_lib} _upperlib)
    if(_kdeBootStrapping)
        set(KDE4_${_upperlib}_LIBRARY ${_lib})
        set(KDE4_${_upperlib}_LIBS    ${_lib})
    else()
        set(KDE4_${_upperlib}_LIBRARY ${KDE4_TARGET_PREFIX}${_lib})
        set(KDE4_${_upperlib}_LIBS    ${KDE4_TARGET_PREFIX}${_lib})
    endif()
endforeach()

#####################  provide some options   ##########################################

if(ENABLE_TESTING)
    enable_testing()
endif()

#####################  some more settings   ##########################################

# If we are building ! kdelibs, check where kdelibs are installed.
# If they are installed in a directory which contains "lib64", we default to "64" for LIB_SUFFIX,
# so the current project will by default also go into lib64.
# The same for lib32. Alex
set(_Init_LIB_SUFFIX "")
if ("${KDE4_LIB_DIR}" MATCHES lib64)
   set(_Init_LIB_SUFFIX 64)
endif ("${KDE4_LIB_DIR}" MATCHES lib64)
if ("${KDE4_LIB_DIR}" MATCHES lib32)
   set(_Init_LIB_SUFFIX 32)
endif ("${KDE4_LIB_DIR}" MATCHES lib32)

# FIXME: this should not be needed
macro(_set_fancy variable value)
    set(${variable} "${value}")
    set(${variable} "${value}" CACHE PATH "KDE standard path variable")
endmacro()

set(LIB_SUFFIX "${_Init_LIB_SUFFIX}" CACHE STRING "Define suffix of directory name (32/64)")

# if bootstrap set the variablse now, otherwise they will be set by KDE4Config
if(_kdeBootStrapping)
    _set_fancy(EXEC_INSTALL_PREFIX  "${CMAKE_INSTALL_PREFIX}")
    _set_fancy(SHARE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/share")
    _set_fancy(BIN_INSTALL_DIR      "${EXEC_INSTALL_PREFIX}/bin")
    _set_fancy(SBIN_INSTALL_DIR     "${EXEC_INSTALL_PREFIX}/sbin")
    _set_fancy(LIB_INSTALL_DIR      "${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
    _set_fancy(LIBEXEC_INSTALL_DIR  "${LIB_INSTALL_DIR}/kde4/libexec")
    _set_fancy(INCLUDE_INSTALL_DIR  "${CMAKE_INSTALL_PREFIX}/include")

    _set_fancy(PLUGIN_INSTALL_DIR       "${LIB_INSTALL_DIR}")
    _set_fancy(IMPORTS_INSTALL_DIR      "${PLUGIN_INSTALL_DIR}/kde4/imports")
    _set_fancy(CONFIG_INSTALL_DIR       "${SHARE_INSTALL_PREFIX}/config")
    _set_fancy(DATA_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/apps")
    _set_fancy(ICON_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/icons")
    _set_fancy(KCFG_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/config.kcfg")
    _set_fancy(LOCALE_INSTALL_DIR       "${SHARE_INSTALL_PREFIX}/locale")
    _set_fancy(MIME_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/mimelnk")
    _set_fancy(SERVICES_INSTALL_DIR     "${SHARE_INSTALL_PREFIX}/kde4/services")
    _set_fancy(SERVICETYPES_INSTALL_DIR "${SHARE_INSTALL_PREFIX}/kde4/servicetypes")
    _set_fancy(SOUND_INSTALL_DIR        "${SHARE_INSTALL_PREFIX}/sounds")
    _set_fancy(TEMPLATES_INSTALL_DIR    "${SHARE_INSTALL_PREFIX}/templates")
    _set_fancy(WALLPAPER_INSTALL_DIR    "${SHARE_INSTALL_PREFIX}/wallpapers")
    _set_fancy(AUTOSTART_INSTALL_DIR    "${SHARE_INSTALL_PREFIX}/autostart")

    _set_fancy(XDG_APPS_INSTALL_DIR     "${SHARE_INSTALL_PREFIX}/applications/kde4")
    _set_fancy(XDG_DIRECTORY_INSTALL_DIR "${SHARE_INSTALL_PREFIX}/desktop-directories")
    _set_fancy(XDG_MIME_INSTALL_DIR     "${SHARE_INSTALL_PREFIX}/mime/packages")

    _set_fancy(SYSCONF_INSTALL_DIR      "${CMAKE_INSTALL_PREFIX}/etc")
    _set_fancy(MAN_INSTALL_DIR          "${SHARE_INSTALL_PREFIX}/man")
    _set_fancy(INFO_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/info")
    _set_fancy(DBUS_INTERFACES_INSTALL_DIR "${SHARE_INSTALL_PREFIX}/dbus-1/interfaces")
    _set_fancy(DBUS_SERVICES_INSTALL_DIR "${SHARE_INSTALL_PREFIX}/dbus-1/services")
    _set_fancy(DBUS_SYSTEM_SERVICES_INSTALL_DIR "${SHARE_INSTALL_PREFIX}/dbus-1/system-services")

    _set_fancy(KAUTH_HELPER_PLUGIN_DIR  "${PLUGIN_INSTALL_DIR}/kde4/plugins/kauth/helper")
    _set_fancy(KAUTH_BACKEND_PLUGIN_DIR "${PLUGIN_INSTALL_DIR}/kde4/plugins/kauth/backend")
endif()

# For more documentation see above.
# Later on it will be possible to extend this for installing OSX frameworks
# The COMPONENT Devel argument has the effect that static libraries belong to the
# "Devel" install component. If we use this also for all install() commands
# for header files, it will be possible to install
#   -everything: make install OR cmake -P cmake_install.cmake
#   -only the development files: cmake -DCOMPONENT=Devel -P cmake_install.cmake
#   -everything except the development files: cmake -DCOMPONENT=Unspecified -P cmake_install.cmake
# This can then also be used for packaging with cpack.
set(INSTALL_TARGETS_DEFAULT_ARGS
    RUNTIME DESTINATION "${BIN_INSTALL_DIR}"
    LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
    ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
    COMPONENT Devel
)

##############  add some more default search paths  ###############
#
# the KDE4_xxx_INSTALL_DIR variables are empty when building kdelibs itself
# and otherwise point to the kde4 install dirs

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


######################################################
#  and now the platform specific stuff
######################################################

if(WIN32 OR CYGWIN OR APPLE)
    message(FATAL_ERROR "Windows/Cygwin/Apple is NOT supported.")
endif()

set(_KDE4_PLATFORM_INCLUDE_DIRS)

# skip re-linking during install
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(Q_WS_X11)
   find_package(X11 REQUIRED)
   # UNIX has already set _KDE4_PLATFORM_INCLUDE_DIRS, so append
   set(_KDE4_PLATFORM_INCLUDE_DIRS ${_KDE4_PLATFORM_INCLUDE_DIRS} ${X11_INCLUDE_DIR} )
endif()

if(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME STREQUAL GNU)
    if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
        set(_KDE4_PLATFORM_DEFINITIONS -D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_GNU_SOURCE)
        set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_SHARED_LINKER_FLAGS}")
        set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_MODULE_LINKER_FLAGS}")

        set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--enable-new-dtags ${CMAKE_SHARED_LINKER_FLAGS}")
        set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--enable-new-dtags ${CMAKE_MODULE_LINKER_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags ${CMAKE_EXE_LINKER_FLAGS}")
    elseif(CMAKE_C_COMPILER_ID MATCHES "Intel")
        set(_KDE4_PLATFORM_DEFINITIONS -D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_GNU_SOURCE)
        set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_SHARED_LINKER_FLAGS}")
        set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_MODULE_LINKER_FLAGS}")
    endif()
endif()

set(_KDE4_PLATFORM_DEFINITIONS "${_KDE4_PLATFORM_DEFINITIONS} -D_LARGEFILE64_SOURCE")

check_cxx_source_compiles("
#include <sys/types.h>
 /* Check that off_t can represent 2**63 - 1 correctly.
    We can't simply define LARGE_OFF_T to be 9223372036854775807,
    since some C++ compilers masquerading as C compilers
    incorrectly reject 9223372036854775807.  */
#define LARGE_OFF_T (((off_t) 1 << 62) - 1 + ((off_t) 1 << 62))

  int off_t_is_large[(LARGE_OFF_T % 2147483629 == 721 && LARGE_OFF_T % 2147483647 == 1) ? 1 : -1];
  int main() { return 0; }
" _OFFT_IS_64BIT)

if(NOT _OFFT_IS_64BIT)
    set(_KDE4_PLATFORM_DEFINITIONS "${_KDE4_PLATFORM_DEFINITIONS} -D_FILE_OFFSET_BITS=64")
endif()


############################################################
# compiler specific settings
############################################################

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(KDE4_ENABLE_EXCEPTIONS "-fexceptions -UQT_NO_EXCEPTIONS")
    # Select flags.
    set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g -DNDEBUG -DQT_NO_DEBUG")
    set(CMAKE_C_FLAGS_RELEASE          "-O2 -DNDEBUG -DQT_NO_DEBUG")
    set(CMAKE_C_FLAGS_DEBUG            "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG -DQT_NO_DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "-O2 -DNDEBUG -DQT_NO_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG          "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -Werror=format -Wformat-security -Wundef")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror=format -Wformat-security -Wundef -fno-exceptions -DQT_NO_EXCEPTIONS -fvisibility-inlines-hidden")

    if(CMAKE_SYSTEM_NAME STREQUAL GNU)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pthread")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -pthread")
    endif()

    check_cxx_compiler_flag(-Woverloaded-virtual __KDE_HAVE_W_OVERLOADED_VIRTUAL)
    if(__KDE_HAVE_W_OVERLOADED_VIRTUAL)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
    endif()

    # check that Qt defines Q_DECL_EXPORT as __attribute__ ((visibility("default")))
    # if it doesn't and KDE compiles with hidden default visibiltiy plugins will break
    set(_source "#include <QtCore/QtGlobal>\n int main()\n {\n #ifndef QT_VISIBILITY_AVAILABLE \n #error QT_VISIBILITY_AVAILABLE is not available\n #endif \n }\n")
    set(_source_file ${CMAKE_BINARY_DIR}/CMakeTmp/check_qt_visibility.cpp)
    file(WRITE "${_source_file}" "${_source}")
    set(_include_dirs "-DINCLUDE_DIRECTORIES:STRING=${QT_INCLUDES}")
    try_compile(_compile_result ${CMAKE_BINARY_DIR} ${_source_file} CMAKE_FLAGS "${_include_dirs}" OUTPUT_VARIABLE _compile_output_var)
    if(NOT _compile_result)
        message("${_compile_output_var}")
        message(FATAL_ERROR "Qt compiled without support for -fvisibility=hidden. This will break plugins and linking of some applications. Please fix your Qt installation (try passing --reduce-exports to configure).")
    endif(NOT _compile_result)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(KDE4_ENABLE_EXCEPTIONS "-fexceptions -UQT_NO_EXCEPTIONS")
    # Select flags.
    set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g -DNDEBUG -DQT_NO_DEBUG")
    set(CMAKE_C_FLAGS_RELEASE          "-O2 -DNDEBUG -DQT_NO_DEBUG")
    set(CMAKE_C_FLAGS_DEBUG            "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG -DQT_NO_DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "-O2 -DNDEBUG -DQT_NO_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG          "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -Werror=format -Wformat-security -Wundef")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror=format -Wformat-security -Wundef -fno-exceptions -DQT_NO_EXCEPTIONS -fvisibility-inlines-hidden")

    # At least kdepim exports one function with C linkage that returns a
    # QString in a plugin, but clang does not like that.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage")

    if(CMAKE_SYSTEM_NAME STREQUAL GNU)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pthread")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -pthread")
    endif (CMAKE_SYSTEM_NAME STREQUAL GNU)

    # check that Qt defines Q_DECL_EXPORT as __attribute__ ((visibility("default")))
    # if it doesn't and KDE compiles with hidden default visibiltiy plugins will break
    set(_source "#include <QtCore/QtGlobal>\n int main()\n {\n #ifndef QT_VISIBILITY_AVAILABLE \n #error QT_VISIBILITY_AVAILABLE is not available\n #endif \n }\n")
    set(_source_file ${CMAKE_BINARY_DIR}/CMakeTmp/check_qt_visibility.cpp)
    file(WRITE "${_source_file}" "${_source}")
    set(_include_dirs "-DINCLUDE_DIRECTORIES:STRING=${QT_INCLUDES}")
    try_compile(_compile_result ${CMAKE_BINARY_DIR} ${_source_file} CMAKE_FLAGS "${_include_dirs}" OUTPUT_VARIABLE _compile_output_var)
    if(NOT _compile_result)
        message("${_compile_output_var}")
        message(FATAL_ERROR "Qt compiled without support for -fvisibility=hidden. This will break plugins and linking of some applications. Please fix your Qt installation (try passing --reduce-exports to configure).")
    endif(NOT _compile_result)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(KDE4_ENABLE_EXCEPTIONS "-fexceptions")
    # Select flags.
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
    set(CMAKE_CXX_FLAGS_RELEASE        "-O2 -DNDEBUG -DQT_NO_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG          "-O2 -g -fno-inline -noalign")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g")
    set(CMAKE_C_FLAGS_RELEASE          "-O2 -DNDEBUG -DQT_NO_DEBUG")
    set(CMAKE_C_FLAGS_DEBUG            "-O2 -g -fno-inline -noalign")

    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -ansi -Wall -w1 -Wpointer-arith -fno-common")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi -Wall -w1 -Wpointer-arith -fno-exceptions -fno-common")
endif()

###########    end of platform specific stuff  ##########################

# KDE4Macros.cmake contains all the KDE specific macros
include(${kde_cmake_module_dir}/KDE4Macros.cmake)

# decide whether KDE4 has been found
set(KDE4_FOUND FALSE)
if (KDE4_INCLUDE_DIR
    AND KDE4_LIB_DIR
    AND KDE4_KCFGC_EXECUTABLE
    AND KDE4_INSTALLED_VERSION_OK)
   set(KDE4_FOUND TRUE)
   set(KDE4Internal_FOUND TRUE) # for feature_summary
endif()

macro(KDE4_PRINT_RESULTS)
    # inside kdelibs the include dir and lib dir are internal, not "found"
    if(NOT _kdeBootStrapping)
        if(KDE4_INCLUDE_DIR)
            message(STATUS "Found KDE 4.18 include dir: ${KDE4_INCLUDE_DIR}")
        else()
            message(STATUS "ERROR: unable to find the KDE 4 headers")
        endif()

        if(KDE4_LIB_DIR)
            message(STATUS "Found KDE 4.18 library dir: ${KDE4_LIB_DIR}")
        else()
            message(STATUS "ERROR: unable to find the KDE 4 core library")
        endif()
    endif()

    if(KDE4_KCFGC_EXECUTABLE)
        message(STATUS "Found the KDE4 kconfig_compiler preprocessor: ${KDE4_KCFGC_EXECUTABLE}")
    else()
        message(STATUS "Didn't find the KDE4 kconfig_compiler preprocessor")
    endif()
endmacro()


if(KDE4Internal_FIND_REQUIRED AND NOT KDE4_FOUND)
    #bail out if something wasn't found
    kde4_print_results()
    if (NOT KDE4_INSTALLED_VERSION_OK)
        message(FATAL_ERROR "ERROR: the installed kdelibs version ${KDE_VERSION} is too old, at least version ${KDE_MIN_VERSION} is required")
    endif()

    if(NOT KDE4_KCFGC_EXECUTABLE)
        message(FATAL_ERROR "ERROR: could not detect a usable kconfig_compiler")
    endif()

    message(FATAL_ERROR "ERROR: could NOT find everything required for compiling KDE 4 programs")
endif()

if(NOT KDE4Internal_FIND_QUIETLY)
    kde4_print_results()
endif()

# add the found Qt and KDE include directories to the current include path
# the ${KDE4_INCLUDE_DIR}/KDE directory is for forwarding includes, eg. #include <KMainWindow>
set(KDE4_INCLUDES
   ${KDE4_INCLUDE_DIR}
   ${KDE4_INCLUDE_DIR}/KDE
   ${QT_INCLUDES}
   ${_KDE4_PLATFORM_INCLUDE_DIRS}
)

# Used by kdebug.h: the "toplevel dir" is one level above CMAKE_SOURCE_DIR
get_filename_component(_KDE4_CMAKE_TOPLEVEL_DIR "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)
string(LENGTH "${_KDE4_CMAKE_TOPLEVEL_DIR}" _KDE4_CMAKE_TOPLEVEL_DIR_LENGTH)

set(KDE4_DEFINITIONS
    ${_KDE4_PLATFORM_DEFINITIONS}
    -DQT_NO_CAST_TO_ASCII
    -DQT_DEPRECATED_WARNINGS
    -DQT_USE_QSTRINGBUILDER
    -DQT_USE_FAST_CONCATENATION
    -DQT_USE_FAST_OPERATOR_PLUS
    -D_REENTRANT
    -DKDE4_CMAKE_TOPLEVEL_DIR_LENGTH=${_KDE4_CMAKE_TOPLEVEL_DIR_LENGTH}
)

if(NOT _kde4_uninstall_rule_created)
    set(_kde4_uninstall_rule_created TRUE)
    configure_file(
        "${kde_cmake_module_dir}/kde4_cmake_uninstall.cmake.in"
        "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake" @ONLY
    )
    add_custom_target(uninstall
        COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
    )
endif()

endif(NOT KDE4_FOUND)
