project(kio)

# This applies to all subdirs; let's put kio's dependencies (and own dirs) here, once and for all
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/bookmarks
    ${CMAKE_CURRENT_SOURCE_DIR}/kio
    ${CMAKE_CURRENT_SOURCE_DIR}/kfile
    # for including kio/kio_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/kio
    # e.g. for observer_stub.h
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KDECORE_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kutils/kpasswdstore
    ${CMAKE_BINARY_DIR}/kutils/kpasswdstore
    ${CMAKE_SOURCE_DIR}/kutils/kexiv2
    ${CMAKE_BINARY_DIR}/kutils/kexiv2
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=7000)

add_subdirectory(misc)
add_subdirectory(metadata)
if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

set(kiocore_STAT_SRCS
    kio/authinfo.cpp
    kio/chmodjob.cpp
    kio/connection.cpp
    kio/copyjob.cpp
    kio/deletejob.cpp
    kio/directorysizejob.cpp
    kio/fileundomanager.cpp
    kio/global.cpp
    kio/job.cpp
    kio/jobuidelegate.cpp
    kio/kabstractfileitemactionplugin.cpp
    kio/kacl.cpp
    kio/kbuildsycocaprogressdialog.cpp
    kio/kdesktopfileactions.cpp
    kio/kdirlister.cpp
    kio/kdirmodel.cpp
    kio/kdirnotify.cpp
    kio/kfileitem.cpp
    kio/kfileitemlistproperties.cpp
    kio/kfileitemactions.cpp
    kio/kfileitemdelegate.cpp
    kio/kfilemetainfo.cpp
    kio/kfilemetainfoitem.cpp
    kio/kfilemetadata.cpp
    kio/kmimetypechooser.cpp
    kio/kprotocolmanager.cpp
    kio/kremoteencoding.cpp
    kio/krun.cpp
    kio/kurifilter.cpp
    kio/kurlcompletion.cpp
    kio/khttp.cpp
    kio/netaccess.cpp
    kio/paste.cpp
    kio/pastedialog.cpp
    kio/previewjob.cpp
    kio/renamedialog.cpp
    kio/scheduler.cpp
    kio/skipdialog.cpp
    kio/slavebase.cpp
    kio/slaveconfig.cpp
    kio/slaveinterface.cpp
    kio/thumbcreator.cpp
    kio/thumbsequencecreator.cpp
    kio/udsentry.cpp
    kio/usernotificationhandler.cpp
    kio/clipboardupdater.cpp
    kio/kautomount.cpp
)

qt4_add_dbus_adaptor(kiocore_STAT_SRCS
    kio/org.kde.kio.FileUndoManager.xml
    fileundomanager_p.h KIO::FileUndoManagerPrivate
    fileundomanager_adaptor
    KIOFileUndoManagerAdaptor
)

set(kbookmarks_STAT_SRCS
    bookmarks/kbookmark.cc
    bookmarks/kbookmarkmanager.cc
    bookmarks/kbookmarkmanageradaptor.cc
    bookmarks/kbookmarkmenu.cc
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
    kfile/kicondialog.cpp
    kfile/kimagefilepreview.cpp
    kfile/kmetaprops.cpp
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
)

########### next target ###############

set(kio_LIB_SRCS
   ${kiocore_STAT_SRCS}
   ${kbookmarks_STAT_SRCS}
   ${kfile_STAT_SRCS}
)

add_library(kio SHARED ${kio_LIB_SRCS})

target_link_libraries(kio PUBLIC
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    kdecore
    kdeui
    kpasswdstore
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
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

install(
    FILES
    application.desktop
    kcmodule.desktop
    kcmoduleinit.desktop
    kfilemetadataplugin.desktop
    kurifilterplugin.desktop
    kfile/kpropertiesdialogplugin.desktop
    kio/kfileitemactionplugin.desktop
    kfile/kiofilemodule.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)

generate_export_header(kio)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kio_export.h
    kio/connection.h
    kio/slaveinterface.h
    kio/slaveconfig.h
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
    kio/fileundomanager.h
    kio/chmodjob.h
    kio/authinfo.h
    kio/ioslave_defaults.h
    kio/previewjob.h
    kio/thumbcreator.h
    kio/thumbsequencecreator.h
    kio/directorysizejob.h
    kio/renamedialog.h
    kio/skipdialog.h
    kio/udsentry.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/kio
)

install(
    FILES
    kio/kabstractfileitemactionplugin.h
    kio/kabstractviewadapter.h
    kio/kacl.h
    kio/kautomount.h
    kio/kbuildsycocaprogressdialog.h
    kio/kdesktopfileactions.h
    kio/kdirlister.h
    kio/kdirmodel.h
    kio/kdirnotify.h
    kio/kfileitem.h
    kio/kfileitemlistproperties.h
    kio/kfileitemactions.h
    kio/kfileitemdelegate.h
    kio/kfilemetainfo.h
    kio/kfilemetainfoitem.h
    kio/kfilemetadata.h
    kio/kmimetypechooser.h
    kio/kprotocolmanager.h
    kio/kremoteencoding.h
    kio/krun.h
    kio/kurifilter.h
    kio/kurlcompletion.h
    kio/khttp.h
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
    bookmarks/kbookmark.h
    bookmarks/kbookmarkmanager.h
    bookmarks/kbookmarkmenu.h
    bookmarks/kbookmarkdialog.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

install(
    FILES
    kio/org.kde.KDirNotify.xml
    kio/org.kde.kio.FileUndoManager.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)

install(
    FILES
    kfile/images/yes.png
    kfile/images/yespartial.png
    kfile/images/user.png
    kfile/images/user-grey.png
    kfile/images/group.png
    kfile/images/group-grey.png
    kfile/images/others.png
    kfile/images/others-grey.png
    kfile/images/mask.png
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/kio/pics
)