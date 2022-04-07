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
#  KDE4_LIB_INSTALL_DIR     - the directory where libraries from kdelibs are installed
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
# The following variables contain all of the depending libraries:
#
#  KDE4_KDECORE_LIBS          - the kdecore library and all depending libraries
#  KDE4_KDEUI_LIBS            - the kdeui library and all depending libraries
#  KDE4_KIO_LIBS              - the kio library and all depending libraries
#  KDE4_KPARTS_LIBS           - the kparts library and all depending libraries
#  KDE4_KIDLETIME_LIBS        - the kidletime library and all depending libraries
#  KDE4_KCMUTILS_LIBS         - the kcmutils library and all depending libraries
#  KDE4_KFILE_LIBS            - the kfile library and all depending libraries
#  KDE4_KDNSSD_LIBS           - the kdnssd library and all depending libraries
#  KDE4_KPTY_LIBS             - the kpty library and all depending libraries
#  KDE4_SOLID_LIBS            - the solid library and all depending libraries
#  KDE4_KNOTIFYCONFIG_LIBS    - the knotify config library and all depending libraries
#  KDE4_KTEXTEDITOR_LIBS      - the ktexteditor library and all depending libraries
#  KDE4_PLASMA_LIBS           - the plasma library and all depending librairies
#  KDE4_KEXIV2_LIBS           - the kexiv2 library and all depending libraries
#  KDE4_KMEDIAPLAYER_LIBS     - the kmediaplayer library and all depending libraries
#  KDE4_KPASSWDSTORE_LIBS     - the kpasswdstore library and all depending libraries
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
#  KDE4_INSTALL_AUTH_HELPER_FILES ( HELPER_TARGET HELPER_ID HELPER_USER )
#   This macro adds the needed files for an helper executable meant to be used by applications using KAuth.
#   It accepts the helper target, the helper ID (the DBUS name) and the user under which the helper will run on.
#   This macro takes care of generate the needed files, and install them in the right location. This boils down
#   to a DBus policy to let the helper register on the system bus, and a service file for letting the helper
#   being automatically activated by the system bus.
#   *WARNING* You have to install the helper in ${KDE4_LIBEXEC_INSTALL_DIR} to make sure everything will work.
#
#
#  This module allows to depend on a particular minimum version of kdelibs.
#  To acomplish that one should use the appropriate cmake syntax for
#  find_package. For example to depend on kdelibs >= 4.21.0 one should use
#
#  find_package(KDE4 4.21.0 REQUIRED)
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
if(NOT CMAKE_VERSION VERSION_LESS "3.3.0")
    cmake_policy(SET CMP0063 NEW)
endif()
if(NOT CMAKE_VERSION VERSION_LESS "3.10.0")
    cmake_policy(SET CMP0071 OLD)
endif()

# Only do something if it hasn't been found yet
if(NOT KDE4_FOUND)

# get the directory of the current file, used later on in the file
get_filename_component(kde_cmake_module_dir ${CMAKE_CURRENT_LIST_FILE} PATH)

# We may only search for other packages with "REQUIRED" if we are required ourselves.
# This file can be processed either (usually) included in FindKDE4.cmake or
# (when building kdelibs) directly via FIND_PACKAGE(KDE4Internal), that's why
# we have to check for both KDE4_FIND_REQUIRED and KDE4Internal_FIND_REQUIRED.
if(KDE4_FIND_REQUIRED OR KDE4Internal_FIND_REQUIRED)
    set(_REQ_STRING_KDE4 REQUIRED)
endif()

find_package(Katie ${_REQ_STRING_KDE4} 4.11.0)

# Check that we really found everything.
# If KDE4 was searched with REQUIRED, we error out with FATAL_ERROR if something
# wasn't found already above in the other FIND_PACKAGE() calls. If KDE4 was
# searched without REQUIRED and something in the FIND_PACKAGE() calls above
# wasn't found,then we get here and must check that everything has actually
# been found. If something is missing, we must not fail with FATAL_ERROR, but only not set KDE4_FOUND.
if(NOT Katie_FOUND)
    message(STATUS "KDE4 not found, because Katie was not found")
    return()
endif()

# now we are sure we have everything we need
include(MacroLibrary)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
# used to be included in MacroLogFeature which was included by MacroLibrary
# TODO: move to main CMakeLists.txt files at the same time CMP0000 is dealt with
include(FeatureSummary)

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
    # This makes setting the _LIBS variables actually a bit superfluos, since e.g. the kdeui
    # library could now also be used just as "KDE4::kdeui" and still have all their dependent
    # libraries handled correctly. But to keep compatibility and not to change behaviour we
    # set all these variables anyway as seen below. Alex
    include(${kde_cmake_module_dir}/KDELibs4LibraryTargets.cmake)
endif(_kdeBootStrapping)

# Set the various KDE4_FOO_LIBS variables.
# In bootstrapping mode KDE4_TARGET_PREFIX is empty, so e.g. KDE4_KDECORE_LIBS
# will be simply set to "kdecore".
set(_kde_libraries
    kmediaplayer
    kcmutils
    kdeclarative
    kdecore
    kdeui
    kdnssd
    kexiv2
    kpasswdstore
    kfile
    kidletime
    kio
    knotifyconfig
    kparts
    kpty
    ktexteditor
    plasma
    solid
)
foreach(_lib ${_kde_libraries})
    string(TOUPPER ${_lib} _upperlib)
    if(_kdeBootStrapping)
        set(KDE4_${_upperlib}_LIBS ${_lib})
    else()
        set(KDE4_${_upperlib}_LIBS ${KDE4_TARGET_PREFIX}${_lib})
    endif()
endforeach()

#####################  provide some options   ##########################################

if(ENABLE_TESTING)
    enable_testing()
endif()

#####################  some more settings   ##########################################

# if bootstrap set the variables now, otherwise they will be set by KDE4Config
if(_kdeBootStrapping)
    include(GNUInstallDirs)

    set(KDE4_EXEC_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH "KDE installation prefix")
    set(KDE4_SHARE_INSTALL_PREFIX "${CMAKE_INSTALL_FULL_DATADIR}" CACHE PATH "KDE shared data installation prefix")
    set(KDE4_BIN_INSTALL_DIR "${CMAKE_INSTALL_FULL_BINDIR}" CACHE PATH "KDE binaries installation directory")
    set(KDE4_SBIN_INSTALL_DIR "${CMAKE_INSTALL_FULL_SBINDIR}" CACHE PATH "KDE system binaries installation directory")
    set(KDE4_LIB_INSTALL_DIR "${CMAKE_INSTALL_FULL_LIBDIR}" CACHE PATH "KDE libraries installation directory")
    set(KDE4_LIBEXEC_INSTALL_DIR "${CMAKE_INSTALL_FULL_LIBEXECDIR}/kde4" CACHE PATH "KDE libraries executables installation directory")
    set(KDE4_INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}" CACHE PATH "KDE headers installation directory")

    set(KDE4_PLUGIN_INSTALL_DIR "${KDE4_LIB_INSTALL_DIR}" CACHE PATH "KDE plugins installation directory")
    set(KDE4_IMPORTS_INSTALL_DIR "${KDE4_PLUGIN_INSTALL_DIR}/kde4/imports" CACHE PATH "KDE imports installation directory")
    set(KDE4_CONFIG_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/config" CACHE PATH "KDE config installation directory")
    set(KDE4_DATA_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/apps" CACHE PATH "KDE data installation directory")
    set(KDE4_ICON_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/icons" CACHE PATH "KDE icon installation directory")
    set(KDE4_KCFG_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/config.kcfg" CACHE PATH "KDE kcfg installation directory")
    set(KDE4_LOCALE_INSTALL_DIR "${CMAKE_INSTALL_FULL_LOCALEDIR}" CACHE PATH "KDE locale installation directory")
    set(KDE4_SERVICES_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/kde4/services" CACHE PATH "KDE services installation directory")
    set(KDE4_SERVICETYPES_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/kde4/servicetypes" CACHE PATH "KDE service types installation directory")
    set(KDE4_SOUND_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/sounds" CACHE PATH "KDE sounds installation directory")
    set(KDE4_TEMPLATES_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/templates" CACHE PATH "KDE templates installation directory")
    set(KDE4_WALLPAPER_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/wallpapers" CACHE PATH "KDE wallpapers installation directory")
    set(KDE4_AUTOSTART_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/autostart" CACHE PATH "KDE autostart installation directory")

    set(KDE4_XDG_APPS_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/applications/kde4" CACHE PATH "KDE XDG applications installation directory")
    set(KDE4_XDG_DIRECTORY_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/desktop-directories" CACHE PATH "KDE XDG directories installation directory")
    set(KDE4_XDG_MIME_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/mime/packages" CACHE PATH "KDE XDG MIME packages installation directory")

    set(KDE4_SYSCONF_INSTALL_DIR "${CMAKE_INSTALL_FULL_SYSCONFDIR}" CACHE PATH "KDE system config installation directory")
    set(KDE4_DBUS_INTERFACES_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/dbus-1/interfaces" CACHE PATH "KDE D-Bus interfaces installation directory")
    set(KDE4_DBUS_SERVICES_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/dbus-1/services" CACHE PATH "KDE D-Bus services installation directory")
    set(KDE4_DBUS_SYSTEM_SERVICES_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/dbus-1/system-services" CACHE PATH "KDE D-Bus system services installation directory")

    set(KDE4_KAUTH_HELPER_PLUGIN_DIR "${KDE4_PLUGIN_INSTALL_DIR}/kde4/plugins/kauth/helper" CACHE PATH "KDE authorization helper installation directory")
    set(KDE4_KAUTH_BACKEND_PLUGIN_DIR "${KDE4_PLUGIN_INSTALL_DIR}/kde4/plugins/kauth/backend" CACHE PATH "KDE authorization backend installation directory")
endif()

# For more documentation see above.
# The COMPONENT Devel argument has the effect that static libraries belong to the
# "Devel" install component. If we use this also for all install() commands
# for header files, it will be possible to install
#   -everything: make install OR cmake -P cmake_install.cmake
#   -only the development files: cmake -DCOMPONENT=Devel -P cmake_install.cmake
#   -everything except the development files: cmake -DCOMPONENT=Unspecified -P cmake_install.cmake
# This can then also be used for packaging with cpack.
set(INSTALL_TARGETS_DEFAULT_ARGS
    RUNTIME DESTINATION "${KDE4_BIN_INSTALL_DIR}"
    LIBRARY DESTINATION "${KDE4_LIB_INSTALL_DIR}"
    ARCHIVE DESTINATION "${KDE4_LIB_INSTALL_DIR}"
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

# mostly for compilers (e.g. Clang) and linkers (e.g. LLDB) with no clue what
# default search path is
include_directories(${KDE4_INCLUDE_INSTALL_DIR})
link_directories(${KDE4_LIB_INSTALL_DIR})

######################################################
#  and now the platform specific stuff
######################################################

if(WIN32 OR CYGWIN OR APPLE)
    message(FATAL_ERROR "Windows/Cygwin/Apple is NOT supported.")
endif()

set(_KDE4_PLATFORM_INCLUDE_DIRS)
set(_KDE4_PLATFORM_DEFINITIONS)

if(Q_WS_X11)
   find_package(X11 REQUIRED)
   # UNIX has already set _KDE4_PLATFORM_INCLUDE_DIRS, so append
   set(_KDE4_PLATFORM_INCLUDE_DIRS ${_KDE4_PLATFORM_INCLUDE_DIRS} ${X11_INCLUDE_DIR} )
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_KDE4_PLATFORM_DEFINITIONS "${_KDE4_PLATFORM_DEFINITIONS} -DNDEBUG")
endif()

############################################################
# compiler specific settings
############################################################

set(KDE4_ENABLE_EXCEPTIONS "-fexceptions -UQT_NO_EXCEPTIONS")

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
            message(STATUS "Found KDE 4.21 include dir: ${KDE4_INCLUDE_DIR}")
        else()
            message(STATUS "ERROR: unable to find the KDE 4 headers")
        endif()

        if(KDE4_LIB_DIR)
            message(STATUS "Found KDE 4.21 library dir: ${KDE4_LIB_DIR}")
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
