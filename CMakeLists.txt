project(kdelibs)

include(GNUInstallDirs)
include(GenerateExportHeader)
include(FeatureSummary)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

################# set KDE specific information #################

set(KDE_VERSION_MAJOR 4)
set(KDE_VERSION_MINOR 24)
set(KDE_VERSION_RELEASE 0)
set(KDE_VERSION "${KDE_VERSION_MAJOR}.${KDE_VERSION_MINOR}.${KDE_VERSION_RELEASE}")
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

# enter bootstrap mode
set(_kdeBootStrapping TRUE)

set(KDE4_TARGET_PREFIX KDE4::)
set(KDE4_KCFGC_EXECUTABLE             kconfig_compiler${CMAKE_EXECUTABLE_SUFFIX})
set(KDE4_MAKEKDEWIDGETS_EXECUTABLE    makekdewidgets${CMAKE_EXECUTABLE_SUFFIX})
set(KDE4_ENABLE_EXCEPTIONS "-fexceptions -UQT_NO_EXCEPTIONS")

set(KDE_DEFAULT_HOME ".katana" CACHE STRING "The default home directory" )
set(KDE4_EXEC_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH "KDE installation prefix")
set(KDE4_SHARE_INSTALL_PREFIX "${CMAKE_INSTALL_FULL_DATADIR}" CACHE PATH "KDE shared data installation prefix")
set(KDE4_BIN_INSTALL_DIR "${CMAKE_INSTALL_FULL_BINDIR}" CACHE PATH "KDE binaries installation directory")
set(KDE4_SBIN_INSTALL_DIR "${CMAKE_INSTALL_FULL_SBINDIR}" CACHE PATH "KDE system binaries installation directory")
set(KDE4_LIB_INSTALL_DIR "${CMAKE_INSTALL_FULL_LIBDIR}" CACHE PATH "KDE libraries installation directory")
set(KDE4_LIBEXEC_INSTALL_DIR "${CMAKE_INSTALL_FULL_LIBEXECDIR}/kde4" CACHE PATH "KDE libraries executables installation directory")
set(KDE4_INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}" CACHE PATH "KDE headers installation directory")
set(KDE4_PLUGIN_INSTALL_DIR "${KDE4_LIB_INSTALL_DIR}/kde4" CACHE PATH "KDE plugins installation directory")
set(KDE4_IMPORTS_INSTALL_DIR "${KDE4_PLUGIN_INSTALL_DIR}/imports" CACHE PATH "KDE imports installation directory")
set(KDE4_CONFIG_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/config" CACHE PATH "KDE config installation directory")
set(KDE4_DATA_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}" CACHE PATH "KDE data installation directory")
set(KDE4_ICON_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/icons" CACHE PATH "KDE icon installation directory")
set(KDE4_LOCALE_INSTALL_DIR "${KDE4_SHARE_INSTALL_PREFIX}/translations/kde4" CACHE PATH "KDE locale installation directory")
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

set(KDE4_DEFINITIONS
    -DQT_NO_CAST_TO_ASCII
    -DQT_DEPRECATED_WARNINGS
)

include(KDE4Defaults)
include(KDE4Macros)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DNDEBUG)
endif()

configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/modules/kde4_exec.sh.in"
    "${CMAKE_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/kde4_exec.sh"
    @ONLY
)
file(
    COPY "${CMAKE_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/kde4_exec.sh"
    DESTINATION "${CMAKE_BINARY_DIR}"
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ
)

################# now find all used packages #################

if(ENABLE_TESTING)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads)
    set_package_properties(Threads PROPERTIES
        DESCRIPTION "Platform dependent threads implementation"
        TYPE REQUIRED
        PURPOSE "Required for asynchorouns functions and methods execution"
    )
endif()

# required packages
find_package(Katie 4.14.0)
set_package_properties(Katie PROPERTIES
    DESCRIPTION "C++ toolkit derived from the Qt 4.8 framework"
    URL "https://bitbucket.org/smil3y/workspace/projects/KATANA"
    TYPE REQUIRED
)

find_package(X11)
set_package_properties(X11 PROPERTIES
    DESCRIPTION "Open source implementation of the X Window System"
    URL "https://xorg.freedesktop.org/wiki/"
    TYPE REQUIRED
)

find_package(LibDeflate)
set_package_properties(LibDeflate PROPERTIES
    DESCRIPTION "Heavily optimized library for DEFLATE/zlib/gzip compression and decompression"
    URL "https://github.com/ebiggers/libdeflate"
    TYPE REQUIRED
    PURPOSE "Compression and decompression of DEFLATE/zlib/gzip format"
)

find_package(SharedMimeInfo 0.91)
set_package_properties(SharedMimeInfo PROPERTIES
    DESCRIPTION "Shared Mime Info"
    URL "http://freedesktop.org/wiki/Software/shared-mime-info"
    TYPE REQUIRED
    PURPOSE "Allows KDE applications to determine file types"
)

find_package(ENCHANT 1.4.0)
set_package_properties(ENCHANT PROPERTIES
    DESCRIPTION "Generic spell checking library"
    URL "https://www.abisource.com/projects/enchant/"
    TYPE REQUIRED
    PURPOSE "Spell checking support via Enchant"
)

# v7.43.0+ required for CURL_AT_LEAST_VERSION() macro
find_package(CURL 7.43.0)
set_package_properties(CURL PROPERTIES
    DESCRIPTION "Command line tool and library for transferring data with URLs"
    URL "https://curl.se/"
    TYPE REQUIRED
    PURPOSE "E-mail library and KIO HTTP(S) slave"
)

# X11 Session Management (SM) is required, X11_SM_FOUND is set in FindX11
if(NOT X11_SM_FOUND)
    message(FATAL_ERROR "The X11 Session Management (SM) development package could not be found")
endif()

if(NOT X11_XSync_FOUND AND NOT X11_Xscreensaver_FOUND)
    message(FATAL_ERROR "Neither the XSync (libXext) nor XScreensaver (libXss) development package was found")
endif()

# optional features
kde4_bool_to_01(X11_XTest_FOUND HAVE_XTEST)
kde4_bool_to_01(X11_Xcursor_FOUND HAVE_XCURSOR)
kde4_bool_to_01(X11_Xfixes_FOUND HAVE_XFIXES)
kde4_bool_to_01(X11_Xscreensaver_FOUND HAVE_XSCREENSAVER)
kde4_bool_to_01(X11_XSync_FOUND HAVE_XSYNC)

add_feature_info("X Test Extension (libXTest)" HAVE_XTEST "Support for notfication restriction")
add_feature_info("X Cursor Extension (Xcursor)" HAVE_XCURSOR "Support for loading cursors depending on theme")
add_feature_info("X Fixes Extension (libXfixes)" HAVE_XFIXES "Support for damage/fixes cursor and window management")
add_feature_info("X Screensaver Extension (libXss)" HAVE_XSCREENSAVER "Support for KIdleTime (fallback mode)")
add_feature_info("X Sync Extension (libXext)" HAVE_XSYNC "Efficient operation of KIdleTime")

# optional packages
kde4_optional_find_package(MPV 0.23.0)
set_package_properties(MPV PROPERTIES
    DESCRIPTION "A fork of mplayer2 and MPlayer"
    URL "https://mpv.io/"
    TYPE RECOMMENDED
    PURPOSE "Media preview, sound notifications, etc."
)

kde4_optional_find_package(ACL)
set_package_properties(ACL PROPERTIES
    DESCRIPTION "Support for manipulating access control lists"
    URL "ftp://oss.sgi.com/projects/xfs/cmd_tars"
    TYPE RECOMMENDED
    PURPOSE "KIO and KIO slaves"
)

kde4_optional_find_package(Avahi)
set_package_properties(Avahi PROPERTIES
    DESCRIPTION "Facilities for service discovery on a local network (DNSSD)"
    URL "https://avahi.org/"
    TYPE OPTIONAL
    PURPOSE "Multicast DNS/DNS-SD service discovery"
)

kde4_optional_find_package(BZip2)
set_package_properties(BZip2 PROPERTIES
    DESCRIPTION "A high-quality data compression program"
    URL "http://www.bzip.org"
    TYPE RECOMMENDED
    PURPOSE "Support for BZip2 compressed files"
)

kde4_optional_find_package(LibLZMA)
set_package_properties(LibLZMA PROPERTIES
    DESCRIPTION "Free general-purpose data compression software with a high compression ratio"
    URL "https://tukaani.org/xz/"
    TYPE OPTIONAL
    PURPOSE "Support for XZ and LZMA compressed files"
)

kde4_optional_find_package(WebP)
set_package_properties(WebP PROPERTIES
    DESCRIPTION "A new image format for the Web"
    URL "https://developers.google.com/speed/webp"
    PURPOSE "Support for WebP image format"
    TYPE OPTIONAL
)

kde4_optional_find_package(LibRaw)
set_package_properties(LibRaw PROPERTIES
    DESCRIPTION "Simple and unified interface for extracting out of RAW files"
    URL "https://www.libraw.org/"
    PURPOSE "Support for RAW image format"
    TYPE OPTIONAL
)

kde4_optional_find_package(LibJPEG)
set_package_properties(LibJPEG PROPERTIES
    DESCRIPTION "JPEG image codec that uses SIMD instructions"
    URL "https://libjpeg-turbo.org/"
    PURPOSE "Support for JPEG image format"
    TYPE OPTIONAL
)

kde4_optional_find_package(OpenJPEG)
set_package_properties(OpenJPEG PROPERTIES
    DESCRIPTION "Open-source JPEG 2000 codec written in C language"
    URL "https://www.openjpeg.org/"
    PURPOSE "Support for JPEG 2000 image format"
    TYPE OPTIONAL
)

kde4_optional_find_package(TIFF)
set_package_properties(TIFF PROPERTIES
    DESCRIPTION "TIFF Library and Utilities"
    URL "http://www.libtiff.org/"
    PURPOSE "Support for TIFF image format"
    TYPE OPTIONAL
)

# v143+ required for udev_monitor_filter_add_match_subsystem_devtype()
kde4_optional_find_package(UDev 143)
set_package_properties(UDev PROPERTIES
    DESCRIPTION "UDev support for Solid"
    URL "http://www.kernel.org/pub/linux/utils/kernel/hotplug/udev.html"
    PURPOSE "Allows Solid to use UDev to provide information about devices on Linux"
    TYPE OPTIONAL
)

kde4_optional_find_package(Devinfo)
set_package_properties(Devinfo PROPERTIES
    DESCRIPTION "Devinfo support for Solid"
    URL "https://www.freebsd.org/cgi/man.cgi?query=devinfo&sektion=3"
    PURPOSE "Allows Solid to use Devinfo to provide information about devices on FreeBSD"
    TYPE OPTIONAL
)

kde4_optional_find_package(LibCDIO)
set_package_properties(LibCDIO PROPERTIES
    DESCRIPTION "GNU Compact Disc Input and Control Library"
    URL "https://www.gnu.org/software/libcdio/"
    PURPOSE "Allows Solid to use libcdio to provide information about CD-ROM devices on Linux"
    TYPE OPTIONAL
)

kde4_optional_find_package(Exiv2 0.21)
set_package_properties(Exiv2 PROPERTIES
    DESCRIPTION "Image metadata library and tools"
    URL "http://www.exiv2.org"
    TYPE RECOMMENDED
    PURPOSE "Exiv2 metadata extraction and image rotation based on the data"
)

kde4_optional_find_package(FFmpeg)
set_package_properties(FFmpeg PROPERTIES
    DESCRIPTION "A complete, cross-platform solution to record, convert and stream audio and video"
    URL "https://ffmpeg.org/"
    TYPE RECOMMENDED
    PURPOSE "Video metadata extraction"
)

kde4_optional_find_package(Taglib)
set_package_properties(Taglib PROPERTIES
    DESCRIPTION "TagLib Audio Meta-Data Library"
    URL "https://taglib.org/"
    TYPE RECOMMENDED
    PURPOSE "Audio metadata extraction"
)

kde4_optional_find_package(EPub)
set_package_properties(EPub PROPERTIES
    DESCRIPTION "A library for reading EPub documents"
    URL "https://sourceforge.net/projects/ebook-tools/"
    TYPE RECOMMENDED
    PURPOSE "eBook metadata extraction"
)

kde4_optional_find_package(Poppler)
set_package_properties(Poppler PROPERTIES
    DESCRIPTION "PDF rendering library"
    URL "https://poppler.freedesktop.org/"
    TYPE RECOMMENDED
    PURPOSE "PDF metadata extraction"
)

kde4_optional_find_package(LibSpectre)
set_package_properties(LibSpectre PROPERTIES
    DESCRIPTION "Small library for rendering Postscript documents"
    URL "https://www.freedesktop.org/wiki/Software/libspectre/"
    TYPE RECOMMENDED
    PURPOSE "Postscript metadata extraction"
)

kde4_optional_find_package(Freetype)
set_package_properties(Freetype PROPERTIES
    DESCRIPTION "Freely available software library to render fonts"
    URL "https://www.freetype.org"
    TYPE RECOMMENDED
    PURPOSE "Fonts metadata extraction"
)

kde4_optional_find_package(DjVuLibre)
set_package_properties(DjVuLibre PROPERTIES
    DESCRIPTION "Open source DjVu library"
    URL "https://djvu.sourceforge.net/"
    TYPE RECOMMENDED
    PURPOSE "DjVu metadata extraction"
)

kde4_optional_find_package(OpenSSL)
set_package_properties(OpenSSL PROPERTIES
    DESCRIPTION "Robust, commercial-grade, full-featured toolkit for general-purpose cryptography and secure communication"
    URL "https://www.openssl.org/"
    TYPE RECOMMENDED
    PURPOSE "Store password securely"
)

kde4_optional_find_package(LibArchive 3.0.3)
set_package_properties(LibArchive PROPERTIES
    DESCRIPTION "Multi-format archive and compression library"
    URL "https://libarchive.org/"
    TYPE RECOMMENDED
    PURPOSE "Archives in TAR, ZIP and other formats"
)

################# configure checks and create the configured files #################

include(ConfigureChecks.cmake)

# now create config headers
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(config-acl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-acl.h)
configure_file(config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h)
configure_file(config-pty.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-pty.h)
configure_file(kdemacros.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h)

#########################################################################

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

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

# kdecore depends on Katie
set(KDE4_KDECORE_INCLUDES
    ${KDE4_KDECORE_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kdecore
    ${CMAKE_BINARY_DIR}/kdecore
    ${CMAKE_SOURCE_DIR}/kdecore/config
    ${CMAKE_SOURCE_DIR}/kdecore/date
    ${CMAKE_SOURCE_DIR}/kdecore/io
    ${CMAKE_SOURCE_DIR}/kdecore/jobs
    ${CMAKE_SOURCE_DIR}/kdecore/kernel
    ${CMAKE_SOURCE_DIR}/kdecore/services
    ${CMAKE_SOURCE_DIR}/kdecore/localization
    ${CMAKE_SOURCE_DIR}/kdecore/sycoca
    ${CMAKE_SOURCE_DIR}/kdecore/text
    ${CMAKE_SOURCE_DIR}/kdecore/util
    ${CMAKE_BINARY_DIR}/kdecore
    ${QT_INCLUDES}
    ${X11_INCLUDE_DIR}
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
    ${CMAKE_SOURCE_DIR}/kdeui/shortcuts
    ${CMAKE_SOURCE_DIR}/kdeui/spell
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

################# list the subdirectories #################

add_subdirectory( cmake )

add_subdirectory( includes )
add_subdirectory( interfaces )
add_subdirectory( kdeclarative )
add_subdirectory( kdecore )
add_subdirectory( kded )
add_subdirectory( kdeui )
add_subdirectory( kdewidgets )
add_subdirectory( kfile )
add_subdirectory( kimgio )
add_subdirectory( kinit )
add_subdirectory( kio )
add_subdirectory( kioslave )
add_subdirectory( kparts )
add_subdirectory( kpty )
add_subdirectory( kutils )
add_subdirectory( licenses )
add_subdirectory( mimetypes )
add_subdirectory( plasma )
add_subdirectory( solid )

################# write kdelibs package config files which will be installed #################

configure_file(
    KDELibs4Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/KDELibs4Config.cmake
    @ONLY
)
configure_file(
    KDELibs4ConfigVersion.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/KDELibs4ConfigVersion.cmake
    @ONLY
)

################# install files #################

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/KDELibs4Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/KDELibs4ConfigVersion.cmake
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/cmake/KDELibs4
)

install(
    EXPORT kdelibsTargets
    NAMESPACE ${KDE4_TARGET_PREFIX}
    FILE KDELibs4Targets.cmake
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/cmake/KDELibs4
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
