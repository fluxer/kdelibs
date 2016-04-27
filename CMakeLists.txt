project(kdelibs)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

################# set KDE specific information #################

set(KDE_VERSION_MAJOR 4)
set(KDE_VERSION_MINOR 18)
set(KDE_VERSION_RELEASE 0)
set(KDE_VERSION "${KDE_VERSION_MAJOR}.${KDE_VERSION_MINOR}.${KDE_VERSION_RELEASE}" )
set(KDE_VERSION_STRING "${KDE_VERSION}")

execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE git_revision
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _unused
)
if(git_revision)
    set(KDE_VERSION_STRING "${KDE_VERSION_STRING} (${git_revision})")
endif()

set(KDE_DISTRIBUTION_TEXT "compiled sources" CACHE STRING "Indicate the distribution in bug reports" )

set(KDE_DEFAULT_HOME ".katana" CACHE STRING "The default home directory" )

set(KDE_NO_DEPRECATED TRUE CACHE BOOL "Whether deprecated functionality is desired")

################# now find all used packages #################

find_package(KDE4Internal REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(GenerateExportHeader)

if(UNIX AND Q_WS_X11)
    #X11 Session Management (SM) is required
    #X11_SM_FOUND is set in FindX11, which is required by KDE4Internal
    if(NOT X11_SM_FOUND)
        message(FATAL_ERROR "\nThe X11 Session Management (SM) development package could not be found.\nPlease install libSM.\n")
    endif(NOT X11_SM_FOUND)
    find_package(XCB)
    if(NOT XCB_FOUND)
        message(FATAL_ERROR "\nThe XCB development package could not be found.\nPlease install libxcb.\n")
    endif(NOT XCB_FOUND)
endif(UNIX AND Q_WS_X11)

# required features:
find_package(Perl)
set_package_properties(Perl PROPERTIES
    URL "http://www.perl.org"
    TYPE RUNTIME
    PURPOSE "Needed for KIO fileshareset and KDEUI preparetips scripts"
)

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
    DESCRIPTION "Support for gzip compressed files and data streams"
    URL "http://www.zlib.net"
    TYPE REQUIRED
    PURPOSE "Required by the core KDE libraries and some critical kioslaves"
)

find_package(SharedMimeInfo 0.60)
set_package_properties(SharedMimeInfo PROPERTIES
    DESCRIPTION "Shared Mime Info"
    URL "http://freedesktop.org/wiki/Software/shared-mime-info"
    TYPE REQUIRED
    PURPOSE "Allows KDE applications to determine file types"
)

find_package(MPV)
set_package_properties(MPV PROPERTIES
    DESCRIPTION "A fork of mplayer2 and MPlayer"
    URL "https://mpv.io/"
    TYPE REQUIRED
)

#optional features
if(X11_FOUND)
    #X11_Xrender discovery is done by FindX11
    add_feature_info("X Rendering Extension (libXrender)" X11_Xrender_FOUND "Support for compositing, rendering operations, and alpha-blending. STRONGLY RECOMMENDED")
    macro_bool_to_01(X11_Xscreensaver_FOUND HAVE_XSCREENSAVER)
    macro_bool_to_01(X11_XSync_FOUND HAVE_XSYNC)
    add_feature_info("X Screensaver Extension (libXss)" HAVE_XSCREENSAVER "Support for KIdleTime (fallback mode)")
    add_feature_info("X Sync Extension (libXext)" HAVE_XSYNC "Efficient operation of KIdleTime. STRONGLY RECOMMENDED")

    if(NOT HAVE_XSYNC AND NOT HAVE_XSCREENSAVER)
        message(FATAL_ERROR "\nNeither the XSync (libXext) nor XScreensaver (libXss) development package was found.\nPlease install one of them (XSync is recommended)\n")
    endif(NOT HAVE_XSYNC AND NOT HAVE_XSCREENSAVER)
endif(X11_FOUND)

macro_optional_find_package(OpenSSL)
set_package_properties(OpenSSL PROPERTIES
    DESCRIPTION "Support for secure network communications (SSL and TLS)"
    URL "http://openssl.org"
    TYPE RECOMMENDED
    PURPOSE "For the bulk of secure communications, including secure web browsing via HTTPS"
)

macro_optional_find_package(Libintl)
set_package_properties(Libintl PROPERTIES
    DESCRIPTION "Support for multiple languages"
    URL "http://www.gnu.org/software/gettext"
    TYPE RECOMMENDED
    PURPOSE "Enables KDE to be available in many different languages"
)

macro_optional_find_package(DBusMenuQt)
set_package_properties(DBusMenuQt PROPERTIES
    DESCRIPTION "Support for notification area menus via the DBusMenu protocol"
    URL "https://launchpad.net/libdbusmenu-qt"
    TYPE RECOMMENDED
)

macro_optional_find_package(Strigi 0.6.3)
set_package_properties(Strigi PROPERTIES
    DESCRIPTION "Desktop indexing and search support"
    URL "http://strigi.sourceforge.net"
    TYPE RECOMMENDED
    PURPOSE "Required by some critical kioslaves"
)

macro_optional_find_package(FAM)
set_package_properties(FAM PROPERTIES
    DESCRIPTION "File alteration notification support via a separate service"
    URL "http://oss.sgi.com/projects/fam"
    TYPE OPTIONAL
    PURPOSE "Provides file alteration notification facilities using a separate service."
)

################# Disallow in-source build #################

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(SEND_ERROR "kdelibs requires an out of source build")
endif()

#########################################################################

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
remove_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)

################# setup the include directories #################

# for including config.h and for includes like <kparts/foo.h>
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
)

# Those variables for  are only valid inside of kdelibs, of course.
# Use the one variable for the lib you depend upon.
# E.g. kdeui uses ${KDE4_KDECORE_INCLUDES}. Something that depends on kparts uses ${KDE4_KPARTS_INCLUDES}.

# kdecore depends on Qt
set(KDE4_KDECORE_INCLUDES
    ${KDE4_KDECORE_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kdecore
    ${CMAKE_BINARY_DIR}/kdecore
    ${CMAKE_SOURCE_DIR}/kdecore/compression
    ${CMAKE_SOURCE_DIR}/kdecore/config
    ${CMAKE_SOURCE_DIR}/kdecore/date
    ${CMAKE_SOURCE_DIR}/kdecore/io
    ${CMAKE_SOURCE_DIR}/kdecore/jobs
    ${CMAKE_SOURCE_DIR}/kdecore/kernel
    ${CMAKE_SOURCE_DIR}/kdecore/auth
    ${CMAKE_SOURCE_DIR}/kdecore/network
    ${CMAKE_SOURCE_DIR}/kdecore/services
    ${CMAKE_SOURCE_DIR}/kdecore/localization
    ${CMAKE_SOURCE_DIR}/kdecore/sycoca
    ${CMAKE_SOURCE_DIR}/kdecore/text
    ${CMAKE_SOURCE_DIR}/kdecore/util
    ${CMAKE_SOURCE_DIR}/kdecore/sonnet
    ${CMAKE_BINARY_DIR}/kdecore
    ${QT_INCLUDES}
    ${_KDE4_PLATFORM_INCLUDE_DIRS}
)

# kdeui depends on kdecore
set(KDE4_KDEUI_INCLUDES
    ${CMAKE_SOURCE_DIR}/kdeui
    ${CMAKE_SOURCE_DIR}/kdeui/actions
    ${CMAKE_SOURCE_DIR}/kdeui/colors
    ${CMAKE_SOURCE_DIR}/kdeui/config
    ${CMAKE_SOURCE_DIR}/kdeui/dialogs
    ${CMAKE_SOURCE_DIR}/kdeui/findreplace
    ${CMAKE_SOURCE_DIR}/kdeui/fonts
    ${CMAKE_SOURCE_DIR}/kdeui/icons
    ${CMAKE_SOURCE_DIR}/kdeui/itemviews
    ${CMAKE_SOURCE_DIR}/kdeui/jobs
    ${CMAKE_SOURCE_DIR}/kdeui/kernel
    ${CMAKE_SOURCE_DIR}/kdeui/notifications
    ${CMAKE_SOURCE_DIR}/kdeui/paged
    ${CMAKE_SOURCE_DIR}/kdeui/plotting
    ${CMAKE_SOURCE_DIR}/kdeui/shortcuts
    ${CMAKE_SOURCE_DIR}/kdeui/sonnet
    ${CMAKE_SOURCE_DIR}/kdeui/util
    ${CMAKE_SOURCE_DIR}/kdeui/widgets
    ${CMAKE_SOURCE_DIR}/kdeui/windowmanagement
    ${CMAKE_SOURCE_DIR}/kdeui/xmlgui
    ${CMAKE_BINARY_DIR}/kdeui
    ${KDE4_KDECORE_INCLUDES}
)

# kio depends on kdeui
set(KDE4_KIO_INCLUDES
    ${CMAKE_SOURCE_DIR}/kio
    ${CMAKE_SOURCE_DIR}/kio/bookmarks
    ${CMAKE_SOURCE_DIR}/kio/kio
    ${CMAKE_SOURCE_DIR}/kio/kfile
    ${CMAKE_BINARY_DIR}/kio
    ${KDE4_KDEUI_INCLUDES}
)

# kpty
set(KDE4_KPTY_INCLUDES
    ${CMAKE_SOURCE_DIR}/kpty
    ${CMAKE_BINARY_DIR}/kpty
    ${KDE4_KIO_INCLUDES}
)

# kparts depends on kio
set(KDE4_KPARTS_INCLUDES
    ${CMAKE_SOURCE_DIR}/kparts
    ${CMAKE_BINARY_DIR}/kparts
    ${KDE4_KIO_INCLUDES}
)

################# configure checks and create the configured files #################

set(LIBRARY_TYPE SHARED)

# ACL stuff (used in kio/ and kioslaves/)
macro_optional_find_package(ACL)
set_package_properties(ACL
    PROPERTIES
    DESCRIPTION "Support for manipulating access control lists"
    URL "ftp://oss.sgi.com/projects/xfs/cmd_tars"
    TYPE RECOMMENDED
    PURPOSE "STRONGLY RECOMMENDED"
)

macro_bool_to_01(ACL_FOUND HAVE_LIBACL HAVE_POSIX_ACL)
configure_file(config-acl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-acl.h)

include(ConfigureChecks.cmake)

# now create config headers
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h)
configure_file(config-compiler.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compiler.h)
configure_file(config-pty.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-pty.h)
configure_file(kdemacros.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h)

################# list the subdirectories #################

add_subdirectory( cmake )

add_subdirectory( dnssd )
add_subdirectory( includes )
add_subdirectory( interfaces )
add_subdirectory( kcddb )
add_subdirectory( kdcraw )
add_subdirectory( kdeclarative )
add_subdirectory( kdecore )
add_subdirectory( kded )
add_subdirectory( kdesu )
add_subdirectory( kdeui )
if(QT_QTWEBKIT_FOUND)
    add_subdirectory( kdewebkit )
endif()
add_subdirectory( kdewidgets )
add_subdirectory( kexiv2 )
add_subdirectory( kfile )
add_subdirectory( kimgio )
add_subdirectory( kinit )
add_subdirectory( kio )
add_subdirectory( kioslave )
add_subdirectory( knotify )
add_subdirectory( kparts )
add_subdirectory( kpty )
add_subdirectory( kross )
add_subdirectory( kunitconversion )
add_subdirectory( kutils )
add_subdirectory( licenses )
add_subdirectory( mimetypes )
add_subdirectory( plasma )
add_subdirectory( security )
add_subdirectory( solid )
add_subdirectory( sonnet )
add_subdirectory( threadweaver )

################# write kdelibs package config files which will be installed #################

macro_bool_to_01(HAVE_UTEMPTER KDE4_KPTY_BUILT_WITH_UTEMPTER)
configure_file(KDE4Config.cmake ${CMAKE_CURRENT_BINARY_DIR}/KDE4Config.cmake @ONLY)
configure_file(KDE4Version.cmake ${CMAKE_CURRENT_BINARY_DIR}/KDE4Version.cmake @ONLY)

################# install files #################

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
)

# the following will be the correct locations once cmake has the improved
# FIND_PACKAGE(): ${SHARE_INSTALL_PREFIX}/cmake/KDE4)
install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/KDE4Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/KDE4Version.cmake
    DESTINATION ${DATA_INSTALL_DIR}/cmake/modules
)

# run a script before installing the exports files which deletes previously
# installed configuration specific export files, if the main exports file has
# changed this makes sure it doesn't include older and different configuration
# specific exports files, which might have a different set of targets or
# targets with different names.
install(
    CODE "set(EXPORT_FILES KDELibs4LibraryTargets.cmake KDELibs4ToolsTargets.cmake)"
    CODE "set(EXPORT_INSTALL_DIR \"${DATA_INSTALL_DIR}/cmake/modules\")"
    SCRIPT "${CMAKE_SOURCE_DIR}/cmake/modules/check_installed_exports_file.cmake"
)
install(
    EXPORT kdelibsLibraryTargets
    NAMESPACE ${KDE4_TARGET_PREFIX}
    FILE KDELibs4LibraryTargets.cmake
    DESTINATION ${DATA_INSTALL_DIR}/cmake/modules
)
install(
    EXPORT kdelibsToolsTargets
    NAMESPACE ${KDE4_TARGET_PREFIX}
    FILE KDELibs4ToolsTargets.cmake
    DESTINATION ${DATA_INSTALL_DIR}/cmake/modules
)

feature_summary(WHAT ALL
    INCLUDE_QUIET_PACKAGES
    FATAL_ON_MISSING_REQUIRED_PACKAGES
)
