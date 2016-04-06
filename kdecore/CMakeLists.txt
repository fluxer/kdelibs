project(kdecore)

# Generate config-kstandarddirs.h
configure_file(
    config-kstandarddirs.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kstandarddirs.h
)

# Generate kdefakes.h
configure_file(
    kdefakes.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/kdefakes.h
)

# Generate kdeversion.h
configure_file(
    util/kdeversion.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/kdeversion.h
)

# Configure checks for kdirwatch
macro_bool_to_01(FAM_FOUND HAVE_FAM)

check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
macro_bool_to_01(SYS_INOTIFY_H_FOUND HAVE_SYS_INOTIFY_H)

# Generate io/config-kdirwatch.h
configure_file(
    io/config-kdirwatch.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/io/config-kdirwatch.h
)

# Configure checks for network/ but also for netsupp.*
include(network/ConfigureChecks.cmake)
configure_file(
    network/config-network.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/network/config-network.h
)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/network)

# Configure checks for date/
include(date/ConfigureChecks.cmake)
configure_file(
    date/config-date.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/date/config-date.h
)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/date)

# Configure checks for compression/
include(compression/ConfigureChecks.cmake)
configure_file(
    compression/config-compression.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/compression/config-compression.h
)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/compression)

# This file handles all the logic for compiling KAuth's backends
include(auth/ConfigureChecks.cmake)

# Configure a small file to tell BackendsManager what to use
configure_file(
    auth/BackendsConfig.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/BackendsConfig.h
)

# Configure checks for localization
configure_file(
    localization/config-localization.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-localization.h
)

# Configure checks for util
include(util/ConfigureChecks.cmake)
configure_file(
    util/config-util.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-util.h
)


include_directories(
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${ZLIB_INCLUDE_DIR}
    ${QT_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/auth
    ${CMAKE_CURRENT_SOURCE_DIR}/sonnet
)

# kdecore_OPTIONAL_SRCS is used to collect source files
# which are not always compiled into kdecore
# if it's used, *always* append to it
set(kdecore_OPTIONAL_SRCS)
# same for optional libs
set(kdecore_OPTIONAL_LIBS)

add_definitions(-DQT_NO_CAST_FROM_ASCII)

# Needed for the kdatetime test in release mode
if(ENABLE_TESTING)
    add_definitions(-DCOMPILING_TESTS)
endif(ENABLE_TESTING)

# compile bzip2 support if available
if(BZIP2_FOUND)
    include_directories(${BZIP2_INCLUDE_DIR})
    set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS} compression/kbzip2filter.cpp)
    set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${BZIP2_LIBRARIES})
endif(BZIP2_FOUND)

# compile lzma support if available
if(LIBLZMA_FOUND)
    include_directories(${LIBLZMA_INCLUDE_DIRS})
    set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS} compression/kxzfilter.cpp)
    set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${LIBLZMA_LIBRARIES})
endif(LIBLZMA_FOUND)

# compile Gettext support if available
if(LIBINTL_FOUND)
    include_directories(${LIBINTL_INCLUDE_DIR})
    set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} ${LIBINTL_LIBRARIES})
endif()

if(HAVE_RESOLV_LIBRARY)
    set(kdecore_OPTIONAL_LIBS ${kdecore_OPTIONAL_LIBS} resolv)
endif()

add_subdirectory(kconfig_compiler)
if(ENABLE_TESTING)
    add_subdirectory(tests)
    add_subdirectory(sonnet/tests)
endif()
add_subdirectory(network/kssld)

########### next target ###############

add_library(kdefakes ${LIBRARY_TYPE} fakes.c)
set_target_properties(kdefakes PROPERTIES
    VERSION ${KDE_NON_GENERIC_LIB_VERSION}
    SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
)
if(NOT HAVE_TRUNC)
    target_link_libraries(kdefakes  m )
endif()

install(
    TARGETS kdefakes
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

########### next target ###############

set(kdecore_LIB_SRCS
    compression/kgzipfilter.cpp
    compression/kfilterbase.cpp
    compression/kfilterdev.cpp
    config/kconfig.cpp
    config/kconfigbase.cpp
    config/kconfigdata.cpp
    config/kconfiggroup.cpp
    config/kconfigini.cpp
    config/kdesktopfile.cpp
    config/ksharedconfig.cpp
    config/kcoreconfigskeleton.cpp
    date/kcalendarera.cpp
    date/kcalendarsystem.cpp
    date/kcalendarsystemcoptic.cpp
    date/kcalendarsystemethiopian.cpp
    date/kcalendarsystemqdate.cpp
    date/kcalendarsystemgregorian.cpp
    date/kcalendarsystemislamiccivil.cpp
    date/kcalendarsystemhebrew.cpp
    date/kcalendarsystemindiannational.cpp
    date/kcalendarsystemjalali.cpp
    date/kcalendarsystemjapanese.cpp
    date/kcalendarsystemjulian.cpp
    date/kcalendarsystemminguo.cpp
    date/kcalendarsystemthai.cpp
    date/kdatetime.cpp
    date/kdatetimeformatter.cpp
    date/kdatetimeparser.cpp
    date/kdayperiod.cpp
    date/klocalizeddate.cpp
    date/ktimezone.cpp
    date/ksystemtimezone.cpp
    date/ktzfiletimezone.cpp
    io/kar.cpp
    io/karchive.cpp
    io/kautosavefile.cpp
    io/kdebug.cpp
    io/kdirwatch.cpp
    io/kfilesystemtype_p.cpp
    io/klimitediodevice.cpp
    io/kmessage.cpp
    io/kmountpoint.cpp
    io/kprocess.cpp
    io/ksavefile.cpp
    io/ktar.cpp
    io/ktempdir.cpp
    io/ktemporaryfile.cpp
    io/kurl.cpp
    io/kzip.cpp
    jobs/kcompositejob.cpp
    jobs/kjob.cpp
    jobs/kjobuidelegate.cpp
    jobs/kjobtrackerinterface.cpp
    kernel/kauthorized.cpp
    kernel/kaboutdata.cpp
    kernel/kcmdlineargs.cpp
    kernel/kdbusconnectionpool.cpp
    kernel/kglobal.cpp
    kernel/kcomponentdata.cpp
    kernel/kstandarddirs.cpp
    kernel/ktoolinvocation.cpp
    auth/kauthaction.cpp
    auth/kauthactionreply.cpp
    auth/kauthactionwatcher.cpp
    auth/AuthBackend.cpp
    auth/BackendsManager.cpp
    auth/HelperProxy.cpp
    auth/kauthhelpersupport.cpp
    services/kfoldermimetype.cpp
    services/kmimetypefactory.cpp
    services/kmimemagicrule.cpp
    services/kmimetypetrader.cpp
    services/kmimetype.cpp
    services/kmimeglobsfileparser.cpp
    services/kmimetyperepository.cpp
    services/kservice.cpp
    services/kserviceaction.cpp
    services/kservicefactory.cpp
    services/kservicegroup.cpp
    services/kservicegroupfactory.cpp
    services/kserviceoffer.cpp
    services/kservicetype.cpp
    services/kservicetypefactory.cpp
    services/kservicetypeprofile.cpp
    services/kservicetypetrader.cpp
    services/ktraderparse.cpp
    services/ktraderparsetree.cpp
    services/yacc.c
    services/lex.c
    services/kplugininfo.cpp
    sonnet/loader.cpp
    sonnet/client.cpp
    sonnet/spellerplugin.cpp
    sonnet/speller.cpp
    sonnet/filter.cpp
    sonnet/settings.cpp
    sonnet/backgroundchecker.cpp
    sonnet/backgroundengine.cpp
    sonnet/globals.cpp

    network/klocalsocket.cpp
    network/ksocketfactory.cpp
    network/ksslcertificatemanager.cpp
    network/ktcpsocket.cpp
    localization/kcatalog.cpp
    localization/kcurrencycode.cpp
    localization/kcharsets.cpp
    localization/kencodingdetector.cpp
    localization/guess_ja.cpp
    localization/klocale.cpp
    localization/klocale_kde.cpp
    localization/klocalizedstring.cpp
    localization/kuitsemantics.cpp
    localization/kuitformats.cpp
    localization/common_helpers.cpp
    sycoca/ksycoca.cpp
    sycoca/ksycocadict.cpp
    sycoca/ksycocaentry.cpp
    sycoca/ksycocafactory.cpp
    sycoca/kprotocolinfo.cpp
    sycoca/kprotocolinfofactory.cpp
    sycoca/kmemfile.cpp
    text/kstringhandler.cpp
    util/kallocator.cpp
    util/kautostart.cpp
    util/kdedmodule.cpp
    util/kdeversion.cpp
    util/klauncher_iface.cpp
    util/kmacroexpander.cpp
    util/kpluginfactory.cpp
    util/kpluginloader.cpp
    util/kshell.cpp
    util/krandom.cpp
    util/krandomsequence.cpp
    util/qtest_kde.cpp

    kernel/ktoolinvocation_x11.cpp
    kernel/kstandarddirs_unix.cpp
    localization/klocale_unix.cpp
    network/klocalsocket_unix.cpp
    network/netsupp.cpp
    io/klockfile_unix.cpp
    util/kshell_unix.cpp
    util/kuser_unix.cpp
    util/kmacroexpander_unix.cpp
    fakes.c

    ${kdecore_OPTIONAL_SRCS}
    ${KAUTH_BACKEND_SRCS}
    ${KAUTH_HELPER_BACKEND_SRCS}
)

if(NOT Q_WS_X11 AND NOT Q_WS_QWS)
    add_definitions(-DNO_DISPLAY)
endif()

add_library(kdecore ${LIBRARY_TYPE} ${kdecore_LIB_SRCS})

target_link_libraries(kdecore PRIVATE
    ${QT_QTCORE_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${ZLIB_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${kdecore_OPTIONAL_LIBS}
)

if(FAM_FOUND)
   include_directories(${FAM_INCLUDE_DIR})
   target_link_libraries(kdecore PRIVATE ${FAM_LIBRARIES})
endif(FAM_FOUND)

target_link_libraries(kdecore PUBLIC
    ${QT_QTDBUS_LIBRARY}
    ${QT_QTCORE_LIBRARY}
)

set_target_properties(kdecore PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS kdecore
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

########### next target ###############

# KAuth backend plugin

set(KAUTH_BACKEND_SRCS ${KAUTH_BACKEND_SRCS} auth/AuthBackend.cpp)
kde4_add_plugin(kauth_backend_plugin ${KAUTH_BACKEND_SRCS})
target_link_libraries(kauth_backend_plugin ${KAUTH_BACKEND_LIBS})
install(
    TARGETS kauth_backend_plugin
    LIBRARY DESTINATION ${KAUTH_BACKEND_PLUGIN_DIR}
    ARCHIVE DESTINATION ${KAUTH_BACKEND_PLUGIN_DIR}
    RUNTIME DESTINATION ${KAUTH_BACKEND_PLUGIN_DIR}
)

########### next target ###############

# KAuth helper plugin

kde4_add_plugin(kauth_helper_plugin ${KAUTH_HELPER_BACKEND_SRCS} auth/HelperProxy.cpp)
target_link_libraries(kauth_helper_plugin ${KAUTH_HELPER_BACKEND_LIBS})
install(
    TARGETS kauth_helper_plugin
    LIBRARY DESTINATION ${KAUTH_HELPER_PLUGIN_DIR}
    ARCHIVE DESTINATION ${KAUTH_HELPER_PLUGIN_DIR}
    RUNTIME DESTINATION ${KAUTH_HELPER_PLUGIN_DIR}
)

########### next target ###############

configure_file(
    localization/all_languages.desktop
    ${CMAKE_CURRENT_BINARY_DIR}/all_languages
    @ONLY
)

add_executable(kde4-config kde-config.cpp )

target_link_libraries(kde4-config ${KDE4_KDECORE_LIBS})

install(TARGETS kde4-config ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

########### install files ###############

install(
    FILES
    localization/entry.desktop
    ${CMAKE_CURRENT_BINARY_DIR}/all_languages
    DESTINATION ${LOCALE_INSTALL_DIR}/en_US
)

install(
    FILES
    kdebug.areas
    kdebugrc
    DESTINATION ${CONFIG_INSTALL_DIR}
)

install(
    FILES
    kdecore_export.h
    compression/kfilterbase.h
    compression/karchive_export.h
    compression/kfilterdev.h
    config/kconfig.h
    config/kconfigbase.h
    config/kconfiggroup.h
    config/kdesktopfile.h
    config/ksharedconfig.h
    config/kcoreconfigskeleton.h
    date/kcalendarsystem.h
    date/kdatetime.h
    date/klocalizeddate.h
    date/ksystemtimezone.h
    date/ktimezone.h
    date/ktzfiletimezone.h
    io/kar.h
    io/karchive.h
    io/kautosavefile.h
    io/kdebug.h
    io/kdirwatch.h
    io/kmessage.h
    io/kprocess.h
    io/klockfile.h
    io/kmountpoint.h
    io/ksavefile.h
    io/ktar.h
    io/ktempdir.h
    io/ktemporaryfile.h
    io/kurl.h
    io/kzip.h
    jobs/kcompositejob.h
    jobs/kjob.h
    jobs/kjobuidelegate.h
    jobs/kjobtrackerinterface.h
    kernel/kaboutdata.h
    kernel/kauthorized.h
    kernel/kcmdlineargs.h
    kernel/kdbusconnectionpool.h
    kernel/kglobal.h
    kernel/kcomponentdata.h
    kernel/kstandarddirs.h
    kernel/ktoolinvocation.h
    network/klocalsocket.h
    network/ksocketfactory.h
    network/ktcpsocket.h
    auth/kauthaction.h
    auth/kauthactionreply.h
    auth/kauthactionwatcher.h
    auth/kauthhelpersupport.h
    auth/kauth.h
    services/kmimetype.h
    services/kmimetypetrader.h
    services/kservice.h
    services/kserviceaction.h
    services/kservicegroup.h
    #services/kservicefactory.h: do not install, internal API
    services/kservicetype.h
    #services/kservicetypefactory.h: do not install, internal API
    services/kservicetypeprofile.h
    services/kservicetypetrader.h
    #services/kserviceoffer.h: do not install, internal API
    services/kplugininfo.h
    localization/kcharsets.h
    localization/kcurrencycode.h
    localization/kencodingdetector.h
    localization/klocale.h
    localization/klocalizedstring.h
    sycoca/kprotocolinfo.h
    sycoca/ksycoca.h
    sycoca/ksycocaentry.h
    sycoca/ksycocatype.h
    text/kstringhandler.h
    util/kallocator.h
    util/kautostart.h
    util/kde_file.h
    util/kdedmodule.h
    util/klauncher_iface.h
    util/kmacroexpander.h
    util/kexportplugin.h
    util/kpluginfactory.h
    util/kpluginloader.h
    util/kshell.h
    util/krandom.h
    util/krandomsequence.h
    util/ksharedptr.h
    util/ksortablelist.h
    util/kuser.h
    util/qtest_kde.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdefakes.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdeversion.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(
    FILES
    sonnet/backgroundchecker.h
    sonnet/speller.h
    sonnet/globals.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/sonnet
    COMPONENT Devel
)

install(
    FILES
    services/kplugininfo.desktop
    sonnet/sonnetspeller.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
)

