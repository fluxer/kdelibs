project(kdecore)

# Generate config-kstandarddirs.h
configure_file(
    config-kstandarddirs.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kstandarddirs.h
)

# Generate kdeversion.h
configure_file(
    util/kdeversion.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/kdeversion.h
)

# Configure checks for date/
include(date/ConfigureChecks.cmake)
configure_file(
    date/config-date.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/date/config-date.h
)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/date)

include_directories(
    ${KDE4_KDECORE_INCLUDES}
    # for kglobalsettings header
    ${KDE4_KDEUI_INCLUDES}
    ${QT_INCLUDES}
)

add_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DKDE_DEFAULT_DEBUG_AREA=180
)

add_subdirectory(kconfig_compiler)
if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

########### next target ###############

set(kdecore_LIB_SRCS
    config/kconfig.cpp
    config/kconfigbase.cpp
    config/kconfigdata.cpp
    config/kconfiggroup.cpp
    config/kconfigini.cpp
    config/kdesktopfile.cpp
    config/ksharedconfig.cpp
    config/kcoreconfigskeleton.cpp
    config/ksettings.cpp
    date/kcalendarera.cpp
    date/kcalendarsystem.cpp
    date/kcalendarsystemqdate.cpp
    date/kcalendarsystemgregorian.cpp
    date/kcalendarsystemjulian.cpp
    date/kdatetime.cpp
    date/kdatetimeformatter.cpp
    date/kdatetimeparser.cpp
    date/kdayperiod.cpp
    date/ktimezone.cpp
    date/ksystemtimezone.cpp
    io/kdebug.cpp
    io/kdirwatch.cpp
    io/kfilesystemtype_p.cpp
    io/kmessage.cpp
    io/kmountpoint.cpp
    io/kprocess.cpp
    io/ksavefile.cpp
    io/ktempdir.cpp
    io/ktemporaryfile.cpp
    io/kurl.cpp
    jobs/kcompositejob.cpp
    jobs/kjob.cpp
    jobs/kjobuidelegate.cpp
    jobs/kjobtrackerinterface.cpp
    kernel/kauthorization.cpp
    kernel/kaboutdata.cpp
    kernel/kcmdlineargs.cpp
    kernel/kdbusconnectionpool.cpp
    kernel/kglobal.cpp
    kernel/kcomponentdata.cpp
    kernel/kstandarddirs.cpp
    kernel/ktoolinvocation.cpp
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
    services/kservicetypetrader.cpp
    services/ktraderparse.cpp
    services/ktraderparsetree.cpp
    services/yacc.c
    services/lex.c
    services/kplugininfo.cpp

    localization/kcatalog.cpp
    localization/kcharsets.cpp
    localization/klocale.cpp
    localization/klocale_kde.cpp
    localization/klocalizedstring.cpp
    localization/kuitsemantics.cpp
    localization/common_helpers.cpp
    sycoca/ksycoca.cpp
    sycoca/ksycocadict.cpp
    sycoca/ksycocaentry.cpp
    sycoca/ksycocafactory.cpp
    sycoca/kprotocolinfo.cpp
    sycoca/kprotocolinfofactory.cpp
    text/kstringhandler.cpp
    util/kautostart.cpp
    util/kde_file.cpp
    util/kdedmodule.cpp
    util/kdevicedatabase.cpp
    util/kdeversion.cpp
    util/kmacroexpander.cpp
    util/kpluginfactory.cpp
    util/kpluginloader.cpp
    util/kshell.cpp
    util/krandom.cpp
    util/kunitconversion.cpp
    util/qtest_kde.cpp

    kernel/ktoolinvocation_x11.cpp
    kernel/kstandarddirs_unix.cpp
    io/klockfile_unix.cpp
    util/kshell_unix.cpp
    util/kuser_unix.cpp
    util/kmacroexpander_unix.cpp
)

add_library(kdecore SHARED ${kdecore_LIB_SRCS})

target_link_libraries(kdecore PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(kdecore PUBLIC
    ${QT_QTCORE_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
)

set_target_properties(kdecore PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS kdecore
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

########### next target ###############

configure_file(
    localization/all_languages.desktop
    ${CMAKE_CURRENT_BINARY_DIR}/all_languages
    @ONLY
)

add_executable(kde4-config kde-config.cpp )

target_link_libraries(kde4-config kdecore)

install(
    TARGETS kde4-config
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)

########### install files ###############

generate_export_header(kdecore)

install(
    FILES
    localization/entry.desktop
    ${CMAKE_CURRENT_BINARY_DIR}/all_languages
    DESTINATION ${KDE4_LOCALE_INSTALL_DIR}
)

install(
    FILES
    kdebug.areas
    kdebugrc
    DESTINATION ${KDE4_CONFIG_INSTALL_DIR}
)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kdecore_export.h
    config/kconfig.h
    config/kconfigbase.h
    config/kconfiggroup.h
    config/kdesktopfile.h
    config/ksharedconfig.h
    config/kcoreconfigskeleton.h
    config/ksettings.h
    date/kcalendarsystem.h
    date/kdatetime.h
    date/ksystemtimezone.h
    date/ktimezone.h
    io/kdebug.h
    io/kdirwatch.h
    io/kmessage.h
    io/kprocess.h
    io/klockfile.h
    io/kmountpoint.h
    io/ksavefile.h
    io/ktempdir.h
    io/ktemporaryfile.h
    io/kurl.h
    jobs/kcompositejob.h
    jobs/kjob.h
    jobs/kjobuidelegate.h
    jobs/kjobtrackerinterface.h
    kernel/kaboutdata.h
    kernel/kauthorization.h
    kernel/kcmdlineargs.h
    kernel/kdbusconnectionpool.h
    kernel/kglobal.h
    kernel/kcomponentdata.h
    kernel/kstandarddirs.h
    kernel/ktoolinvocation.h
    services/kmimetype.h
    services/kmimetypetrader.h
    services/kservice.h
    services/kserviceaction.h
    services/kservicegroup.h
    #services/kservicefactory.h: do not install, internal API
    services/kservicetype.h
    #services/kservicetypefactory.h: do not install, internal API
    services/kservicetypetrader.h
    #services/kserviceoffer.h: do not install, internal API
    services/kplugininfo.h
    localization/kcharsets.h
    localization/klocale.h
    localization/klocalizedstring.h
    sycoca/kprotocolinfo.h
    sycoca/ksycoca.h
    sycoca/ksycocaentry.h
    sycoca/ksycocatype.h
    text/kstringhandler.h
    util/kautostart.h
    util/kde_file.h
    util/kdedmodule.h
    util/kdevicedatabase.h
    util/kmacroexpander.h
    util/kpluginfactory.h
    util/kpluginloader.h
    util/kshell.h
    util/krandom.h
    util/kunitconversion.h
    util/ksharedptr.h
    util/ksortablelist.h
    util/kuser.h
    util/qtest_kde.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdeversion.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    FILES
    services/kplugininfo.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)

install(
    FILES
    pci.ids
    usb.ids
    kde4_pci.ids
    kde4_usb.ids
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/kdevicedatabase
)

install(
    FILES
    dbus_policy.stub
    dbus_service.stub
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/kauth
)
