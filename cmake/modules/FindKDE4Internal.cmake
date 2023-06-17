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
#  KDE4_KPTY_LIBS             - the kpty library and all depending libraries
#  KDE4_SOLID_LIBS            - the solid library and all depending libraries
#  KDE4_KNOTIFYCONFIG_LIBS    - the knotify config library and all depending libraries
#  KDE4_KTEXTEDITOR_LIBS      - the ktexteditor library and all depending libraries
#  KDE4_PLASMA_LIBS           - the plasma library and all depending librairies
#  KDE4_KEXIV2_LIBS           - the kexiv2 library and all depending libraries
#  KDE4_KMEDIAPLAYER_LIBS     - the kmediaplayer library and all depending libraries
#  KDE4_KPASSWDSTORE_LIBS     - the kpasswdstore library and all depending libraries
#  KDE4_KPOWERMANAGER_LIBS    - the kpowermanager library and all depending libraries
#  KDE4_KDNSSD_LIBS           - the kdnssd library and all depending libraries
#  KDE4_KARCHIVE_LIBS         - the karchive library and all depending libraries
#
# The variable INSTALL_TARGETS_DEFAULT_ARGS can be used when installing libraries
# or executables into the default locations.
# The INSTALL_TARGETS_DEFAULT_ARGS variable should be used when libraries are installed.
# It should also be used when installing applications.
# The variable MUST NOT be used for installing plugins.
# It also MUST NOT be used for executables which are intended to go into sbin/ or libexec/.
#
# Usage is like this:
#    install(TARGETS kdecore kdeui ${INSTALL_TARGETS_DEFAULT_ARGS})
#
#  This module allows to depend on a particular minimum version of kdelibs.
#  To acomplish that one should use the appropriate cmake syntax for
#  find_package. For example to depend on kdelibs >= 4.23.0 one should use
#
#  find_package(KDE4 4.23.0 REQUIRED)
#
#  In earlier versions of KDE you could use the variable KDE_MIN_VERSION to
#  have such a dependency. This variable is deprecated with KDE 4.2.0, but
#  will still work to make the module backwards-compatible.

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
    # CMP0003: enable symbols visibility preset for all targets
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

find_package(Katie ${_REQ_STRING_KDE4} 4.13.0)
find_package(X11 REQUIRED)

# Used in configure_file() and install(EXPORT)
set(KDE4_TARGET_PREFIX KDE4::)

#######################  #now try to find some kde stuff  ################################

if(NOT _kdeBootStrapping)
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

    # allow searching cmake modules in the kde install locations
    set(kde_modules_install_dir "${KDE4_DATA_INSTALL_DIR}/cmake/modules")
    if(EXISTS "${kde_modules_install_dir}")
        string(TOLOWER "${kde_modules_install_dir}" _apath)
        # ignore already added pathes, case insensitive
        foreach(adir ${CMAKE_MODULE_PATH})
            string(TOLOWER "${adir}" _adir)
            if(NOT "${_adir}" STREQUAL "${_apath}")
                message(STATUS "Adding ${kde_modules_install_dir} to CMAKE_MODULE_PATH")
                set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${kde_modules_install_dir}")
            endif()
        endforeach()
    endif()

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
endif(NOT _kdeBootStrapping)

# Set the various KDE4_FOO_LIBS variables.
# In bootstrapping mode KDE4_TARGET_PREFIX is empty, so e.g. KDE4_KDECORE_LIBS
# will be simply set to "kdecore".
set(_kde_libraries
    kmediaplayer
    kcmutils
    kdeclarative
    kdecore
    kdeui
    kexiv2
    kpasswdstore
    kpowermanager
    kdnssd
    karchive
    kemail
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

if(ENABLE_TESTING)
    enable_testing()
endif()

#####################  some more settings   ##########################################

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
            message(STATUS "Found KDE 4.23 include dir: ${KDE4_INCLUDE_DIR}")
        else()
            message(STATUS "ERROR: unable to find the KDE 4 headers")
        endif()

        if(KDE4_LIB_DIR)
            message(STATUS "Found KDE 4.23 library dir: ${KDE4_LIB_DIR}")
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

# add the found Katie and KDE include directories to the current include path
# the ${KDE4_INCLUDE_DIR}/KDE directory is for forwarding includes, eg. #include <KMainWindow>
set(KDE4_INCLUDES
   ${KDE4_INCLUDE_DIR}
   ${KDE4_INCLUDE_DIR}/KDE
   ${QT_INCLUDES}
   ${X11_INCLUDE_DIR}
)

set(KDE4_DEFINITIONS
    -DQT_NO_CAST_TO_ASCII
    -DQT_DEPRECATED_WARNINGS
)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(KDE4_DEFINITIONS "${KDE4_DEFINITIONS} -DNDEBUG")
endif()

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
