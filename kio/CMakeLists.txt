project(kio)

if(NOT Strigi_FOUND)
    set(KIO_NO_STRIGI TRUE)
endif()

# This applies to all subdirs; let's put kio's dependencies (and own dirs) here, once and for all
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/bookmarks
    ${CMAKE_CURRENT_SOURCE_DIR}/kio
    ${CMAKE_CURRENT_SOURCE_DIR}/kfile
    ${CMAKE_CURRENT_SOURCE_DIR}/kssl
    # for including kio/kio_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/kio
    ${CMAKE_CURRENT_BINARY_DIR}/kssl
    # e.g. for observer_stub.h
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KDECORE_INCLUDES}
    ${ZLIB_INCLUDE_DIR}
)

if(NOT KIO_NO_STRIGI)
    include_directories(${STRIGI_INCLUDE_DIR})
    set(KIO_EXTRA_LIBS
        ${KIO_EXTRA_LIBS}
        ${STRIGI_STREAMANALYZER_LIBRARY}
        ${STRIGI_STREAMS_LIBRARY}
    )
endif()

configure_file(
    kio/config-kio.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/kio/config-kio.h
)

add_subdirectory(kssl/kcm)
add_subdirectory(misc)
if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

set(kiocore_STAT_SRCS
    kio/accessmanager.cpp
    kio/accessmanagerreply_p.cpp
    kio/authinfo.cpp
    kio/chmodjob.cpp
    kio/connection.cpp
    kio/copyjob.cpp
    kio/dataprotocol.cpp
    kio/dataslave.cpp
    kio/davjob.cpp
    kio/defaultviewadapter.cpp
    kio/deletejob.cpp
    kio/directorysizejob.cpp
    kio/filejob.cpp
    kio/fileundomanager.cpp
    kio/forwardingslavebase.cpp
    kio/global.cpp
    kio/imagefilter.cpp
    kio/job.cpp
    kio/jobuidelegate.cpp
    kio/joburlcache.cpp
    kio/kabstractfileitemactionplugin.cpp
    kio/kacl.cpp
    kio/kbuildsycocaprogressdialog.cpp
    kio/kdatatool.cpp
    kio/kdbusservicestarter.cpp
    kio/kdesktopfileactions.cpp
    kio/kdirlister.cpp
    kio/kdirmodel.cpp
    kio/kdirnotify.cpp
    kio/kdynamicjobtracker.cpp
    kio/kemailsettings.cpp
    kio/kfileitem.cpp
    kio/kfileitemlistproperties.cpp
    kio/kfileitemactions.cpp
    kio/kfileitemdelegate.cpp
    kio/kfilemetainfo.cpp
    kio/kfilemetainfoitem.cpp
    kio/kfileshare.cpp
    kio/kfilewriteplugin.cpp
    kio/kimageio.cpp
    kio/kmimetypechooser.cpp
    kio/kprotocolmanager.cpp
    kio/kremoteencoding.cpp
    kio/krun.cpp
    kio/kscan.cpp
    kio/kshellcompletion.cpp
    kio/kurifilter.cpp
    kio/kurlcompletion.cpp
    kio/metainfojob.cpp
    kio/netaccess.cpp
    kio/paste.cpp
    kio/pastedialog.cpp
    kio/predicateproperties.cpp
    kio/previewjob.cpp
    kio/renamedialog.cpp
    kio/renamedialogplugin.cpp
    kio/scheduler.cpp
    kio/sessiondata.cpp
    kio/skipdialog.cpp
    kio/slave.cpp
    kio/slavebase.cpp
    kio/slaveconfig.cpp
    kio/slaveinterface.cpp
    kio/tcpslavebase.cpp
    kio/thumbcreator.cpp
    kio/thumbsequencecreator.cpp
    kio/udsentry.cpp
    kio/hostinfo.cpp
    kio/kpasswdserver.cpp
    kio/kpasswdserverloop.cpp
    kio/usernotificationhandler.cpp
    kio/clipboardupdater.cpp
)

qt4_add_dbus_adaptor(kiocore_STAT_SRCS
    kio/org.kde.kio.FileUndoManager.xml
    fileundomanager_p.h KIO::FileUndoManagerPrivate
    fileundomanager_adaptor
    KIOFileUndoManagerAdaptor
)

qt4_add_dbus_interface(kiocore_STAT_SRCS
    kio/org.kde.kuiserver.xml
    kuiserver_interface
)

set_source_files_properties(kio/org.kde.KPasswdServer.xml
   PROPERTIES INCLUDE kio/authinfo.h
)
qt4_add_dbus_interface(kiocore_STAT_SRCS
    kio/org.kde.KPasswdServer.xml
    kpasswdserver_interface
)

if(UNIX)
   set(kiocore_STAT_SRCS ${kiocore_STAT_SRCS}
       kio/kautomount.cpp
       kio/knfsshare.cpp
       kio/ksambashare.cpp
       kio/ksambasharedata.cpp
   )
endif(UNIX)

set(kbookmarks_STAT_SRCS
    bookmarks/kbookmark.cc
    bookmarks/kbookmarkimporter.cc
    bookmarks/kbookmarkmanager.cc
    bookmarks/kbookmarkmanageradaptor.cc
    bookmarks/kbookmarkmenu.cc
    bookmarks/konqbookmarkmenu.cc
    bookmarks/kbookmarkimporter_opera.cc
    bookmarks/kbookmarkimporter_ie.cc
    bookmarks/kbookmarkimporter_ns.cc
    bookmarks/kbookmarkdombuilder.cc
    bookmarks/kbookmarkdialog.cc
)

set(kfile_STAT_SRCS
    kfile/kabstractfilemodule.cpp
    kfile/kacleditwidget.cpp
    kfile/kdiskfreespaceinfo.cpp
    kfile/kencodingfiledialog.cpp
    kfile/kfile.cpp
    kfile/kfiledialog.cpp
    kfile/kfilemetadataconfigurationwidget.cpp
    kfile/kfilemetadatawidget.cpp
    kfile/kfilemetainfowidget.cpp
    kfile/kfilemetapreview.cpp
    kfile/kicondialog.cpp
    kfile/kimagefilepreview.cpp
    kfile/kmetaprops.cpp
    kfile/knfotranslator.cpp
    kfile/kopenwithdialog.cpp
    kfile/kpreviewprops.cpp
    kfile/kpreviewwidgetbase.cpp
    kfile/kpropertiesdialog.cpp
    kfile/krecentdirs.cpp
    kfile/krecentdocument.cpp
    kfile/kurlcombobox.cpp
    kfile/kurlrequester.cpp
    kfile/kurlrequesterdialog.cpp
    kfile/kfilemetadataprovider.cpp
    kfile/kfilesharedialog.cpp
)

qt4_add_resources(kfile_STAT_SRCS kfile/kacleditwidget.qrc)

set(kssl_STAT_SRCS
    kssl/sslui.cpp
    # for sslui.cpp
    kssl/ksslcertificatebox.cpp
    kssl/ksslinfodialog.cpp
    kssl/ksslsettings.cpp
)

########### next target ###############

set(kio_LIB_SRCS
   ${kiocore_STAT_SRCS}
   ${kbookmarks_STAT_SRCS}
   ${kfile_STAT_SRCS}
   ${kssl_STAT_SRCS}
)

add_library(kio ${LIBRARY_TYPE} ${kio_LIB_SRCS})

target_link_libraries(kio PRIVATE
    ${ZLIB_LIBRARY}
    ${X11_LIBRARIES}
    ${KIO_EXTRA_LIBS}
)

target_link_libraries(kio PUBLIC
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTSVG_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

if(ACL_FOUND)
    target_link_libraries(kio PRIVATE ${ACL_LIBS})
endif()

set_target_properties(kio PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)


########### install files ###############

install(
    TARGETS kio
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(
    FILES
    data.protocol
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)

install(
    FILES
    application.desktop
    kcmodule.desktop
    kcmoduleinit.desktop
    kdatatool.desktop
    kfileplugin.desktop
    kscan.desktop
    kurifilterplugin.desktop
    renamedialogplugin.desktop
    kfile/kpropertiesdialogplugin.desktop
    kio/kfilewrite.desktop
    kio/kfileitemactionplugin.desktop
    kfile/kiofilemodule.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)

generate_export_header(kio)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kio_export.h
    kio/accessmanager.h
    kio/connection.h
    kio/slaveinterface.h
    kio/slave.h
    kio/slaveconfig.h
    kio/sessiondata.h
    kio/global.h
    kio/netaccess.h
    kio/job.h
    kio/scheduler.h
    kio/jobclasses.h
    kio/copyjob.h
    kio/deletejob.h
    kio/jobuidelegate.h
    kio/paste.h
    kio/slavebase.h
    kio/filejob.h
    kio/fileundomanager.h
    kio/tcpslavebase.h
    kio/forwardingslavebase.h
    kio/chmodjob.h
    kio/authinfo.h
    kio/ioslave_defaults.h
    kio/http_slave_defaults.h
    kio/previewjob.h
    kio/thumbcreator.h
    kio/thumbsequencecreator.h
    kio/metainfojob.h
    kio/davjob.h
    kio/directorysizejob.h
    kio/renamedialog.h
    kio/renamedialogplugin.h
    kio/skipdialog.h
    kio/udsentry.h
    kssl/sslui.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/kio
    COMPONENT Devel
)

install(
    FILES
    kio/kabstractfileitemactionplugin.h
    kio/kacl.h
    kio/kautomount.h
    kio/kbuildsycocaprogressdialog.h
    kio/kdatatool.h
    kio/kdbusservicestarter.h
    kio/kdesktopfileactions.h
    kio/kdirlister.h
    kio/kdirmodel.h
    kio/kdirnotify.h
    kio/kemailsettings.h
    kio/kfileitem.h
    kio/kfileitemlistproperties.h
    kio/kfileitemactions.h
    kio/kfileitemdelegate.h
    kio/kfilemetainfo.h
    kio/kfilemetainfoitem.h
    kio/kfileshare.h
    kio/kfilewriteplugin.h
    kio/kimageio.h
    kio/kmimetypechooser.h
    kio/knfsshare.h
    kio/kprotocolmanager.h
    kio/kremoteencoding.h
    kio/krun.h
    kio/ksambashare.h
    kio/ksambasharedata.h
    kio/kscan.h
    kio/kshellcompletion.h
    kio/kurifilter.h
    kio/kurlcompletion.h
    kio/predicateproperties.h
    kfile/kabstractfilemodule.h
    kfile/kabstractfilewidget.h
    kfile/kdiskfreespaceinfo.h
    kfile/kencodingfiledialog.h
    kfile/kfile.h
    kfile/kfiledialog.h
    kfile/kicondialog.h
    kfile/kimagefilepreview.h
    kfile/kfilemetadataconfigurationwidget.h
    kfile/kfilemetadatawidget.h
    kfile/kopenwithdialog.h
    kfile/kpreviewwidgetbase.h
    kfile/kpropertiesdialog.h
    kfile/krecentdirs.h
    kfile/krecentdocument.h
    kfile/kurlcombobox.h
    kfile/kurlrequester.h
    kfile/kurlrequesterdialog.h
    kfile/kfilesharedialog.h
    bookmarks/kbookmark.h
    bookmarks/kbookmarkexporter.h
    bookmarks/kbookmarkimporter.h
    bookmarks/kbookmarkmanager.h
    bookmarks/kbookmarkmenu.h
    bookmarks/kbookmarkimporter_opera.h
    bookmarks/kbookmarkimporter_ie.h
    bookmarks/kbookmarkimporter_ns.h
    bookmarks/kbookmarkdombuilder.h
    bookmarks/konqbookmarkmenu.h
    bookmarks/kbookmarkdialog.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(
    FILES
    kio/org.kde.KDirNotify.xml
    kio/org.kde.kio.FileUndoManager.xml
    kio/org.kde.KPasswdServer.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)


install(
    FILES
    kio/accept-languages.codes
    DESTINATION ${KDE4_CONFIG_INSTALL_DIR}
)
