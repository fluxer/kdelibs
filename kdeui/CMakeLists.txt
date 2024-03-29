project(kdeui)

include_directories(
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/kdeui
    ${KDE4_KDECORE_INCLUDES}
    actions
    colors
    config
    dialogs
    findreplace
    fonts
    icons
    itemviews
    jobs
    kernel
    notifications
    paged
    shortcuts
    spell
    util
    widgets
    windowmanagement
    xmlgui
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/util
    ${CMAKE_CURRENT_BINARY_DIR}/widgets
    ${ENCHANT_INCLUDE_DIR}
    "${ENCHANT_INCLUDE_DIR}/.."
)

if (X11_Xkb_FOUND AND X11_Xkbfile_FOUND)
     set(HAVE_XKB TRUE)
endif()
configure_file(
    util/config-kkeyserver.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/util/config-kkeyserver.h
)

add_definitions(
    -DKDE_DEFAULT_DEBUG_AREA=240
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

########### next target ###############
set(kdeui_LIB_SRCS
    actions/kactioncollection.cpp
    actions/kactioncategory.cpp
    actions/kaction.cpp
    actions/kactionmenu.cpp
    actions/kcodecaction.cpp
    actions/kdualaction.cpp
    actions/kfontaction.cpp
    actions/kfontsizeaction.cpp
    actions/kpastetextaction.cpp
    actions/krecentfilesaction.cpp
    actions/kselectaction.cpp
    actions/kstandardaction.cpp
    actions/ktoggleaction.cpp
    actions/ktoggletoolbaraction.cpp
    actions/ktogglefullscreenaction.cpp
    actions/ktoolbarlabelaction.cpp
    actions/ktoolbarpopupaction.cpp
    actions/ktoolbarspaceraction.cpp
    colors/kcolorspaces.cpp
    colors/kcolorutils.cpp
    colors/kcolorscheme.cpp
    colors/kcolorcollection.cpp
    colors/kcolorhelpers.cpp
    colors/kcolorbutton.cpp
    colors/kcolorcombo.cpp
    colors/kcolorchoosermode.cpp
    colors/kcolordialog.cpp
    colors/kcolormimedata.cpp
    colors/kcolorvalueselector.cpp
    colors/khuesaturationselect.cpp
    config/kconfigskeleton.cpp
    config/kconfiggroupgui.cpp
    dialogs/kaboutkdedialog_p.cpp
    dialogs/kconfigdialog.cpp
    dialogs/kconfigdialogmanager.cpp
    dialogs/kdialog.cpp
    dialogs/kedittoolbar.cpp
    dialogs/kinputdialog.cpp
    dialogs/klinkdialog.cpp
    dialogs/kmessagebox.cpp
    dialogs/kmessageboxmessagehandler.cpp
    dialogs/kaboutapplicationdialog.cpp
    dialogs/kaboutapplicationpersonmodel_p.cpp
    dialogs/kaboutapplicationpersonlistdelegate_p.cpp
    dialogs/kaboutapplicationpersonlistview_p.cpp
    dialogs/knewpassworddialog.cpp
    dialogs/kpassworddialog.cpp
    dialogs/kprogressdialog.cpp
    dialogs/kshortcuteditwidget.cpp
    dialogs/kshortcutseditordelegate.cpp
    dialogs/kshortcutseditoritem.cpp
    dialogs/kshortcutseditor.cpp
    dialogs/kshortcutschemeseditor.cpp
    dialogs/kshortcutsdialog.cpp
    dialogs/ksplashscreen.cpp
    dialogs/kswitchlanguagedialog_p.cpp
    dialogs/ktimerdialog.cpp
    dialogs/ktip.cpp
    dialogs/kdeprintdialog.cpp
    findreplace/kfind.cpp
    findreplace/kreplace.cpp
    findreplace/kfinddialog.cpp
    findreplace/kreplacedialog.cpp
    fonts/fonthelpers.cpp
    fonts/kfontchooser.cpp
    fonts/kfontcombobox.cpp
    fonts/kfontdialog.cpp
    fonts/kfontrequester.cpp
    fonts/kfontutils.cpp
    icons/kiconeffect.cpp
    icons/kiconengine.cpp
    icons/kicon.cpp
    icons/kiconloader.cpp
    icons/kicontheme.cpp
    itemviews/klinkitemselectionmodel.cpp
    itemviews/krecursivefilterproxymodel.cpp
    itemviews/klistwidget.cpp
    itemviews/klistwidgetsearchline.cpp
    itemviews/ktreewidgetsearchline.cpp
    itemviews/kfilterproxysearchline.cpp
    itemviews/kextendableitemdelegate.cpp
    itemviews/kcategorizedsortfilterproxymodel.cpp
    itemviews/kcategorizedview.cpp
    itemviews/kcategorydrawer.cpp
    itemviews/kmodelindexproxymapper.cpp
    itemviews/kwidgetitemdelegate.cpp
    itemviews/kwidgetitemdelegatepool.cpp
    # This requires patches which are not yet in Qt. Disabled for now.
    # itemviews/kreparentingproxymodel.cpp
    jobs/kdialogjobuidelegate.cpp
    jobs/kabstractwidgetjobtracker.cpp
    jobs/kwidgetjobtracker.cpp
    jobs/kstatusbarjobtracker.cpp
    jobs/kuiserverjobtracker.cpp
    jobs/kdynamicjobtracker.cpp
    kernel/kapplication.cpp
    kernel/kuniqueapplication.cpp
    kernel/ksessionmanager.cpp
    kernel/kstyle.cpp
    kernel/kstartupinfo.cpp
    kernel/kglobalsettings.cpp
    kernel/ksystemeventfilter.cpp
    kernel/ktoolinvocation.cpp
    kernel/ktoolinvocation_x11.cpp
    notifications/knotification.cpp
    notifications/knotificationconfigwidget.cpp
    notifications/knotificationrestrictions.cpp
    notifications/kdbusmenuexporter.cpp
    notifications/kdbusmenuimporter.cpp
    notifications/kdbusmenu_p.cpp
    notifications/ksystemtrayicon.cpp
    notifications/kstatusnotifieritem.cpp
    notifications/kstatusnotifieritemdbus_p.cpp
    paged/kpagedialog.cpp
    paged/kpageview.cpp
    paged/kpageview_p.cpp
    paged/kpagewidget.cpp
    paged/kpagewidgetmodel.cpp
    shortcuts/kshortcut.cpp
    shortcuts/kshortcutschemeshelper.cpp
    shortcuts/kstandardshortcut.cpp
    shortcuts/kglobalaccel.cpp
    shortcuts/kglobalshortcutinfo.cpp
    shortcuts/kglobalshortcutinfo_dbus.cpp
    shortcuts/kacceleratormanager.cpp
    shortcuts/kcheckaccelerators.cpp
    spell/kspeller.cpp
    spell/kspellhighlighter.cpp
    spell/kspelldictionarycombobox.cpp
    spell/kspellconfigwidget.cpp
    util/kcompletion.cpp
    util/kcompletionbase.cpp
    util/kcrash.cpp
    util/kcursor.cpp
    util/kguiitem.cpp
    util/kkeyserver.cpp
    util/kselectionowner.cpp
    util/knumvalidator.cpp
    util/kpassivepopup.cpp
    util/kpassivepopupmessagehandler.cpp
    util/kstandardguiitem.cpp
    util/kxerrorhandler.cpp
    util/kxmessages.cpp
    util/kundostack.cpp
    util/kpixmap.cpp
    util/kpixmapsequence.cpp
    util/kpixmapsequenceoverlaypainter.cpp
    util/kpixmapsequencewidget.cpp
    util/kimageio.cpp
    util/kkeyserver_x11.cpp
    util/kkeyboardlayout.cpp
    widgets/kactionselector.cpp
    widgets/kdialogbuttonbox.cpp
    widgets/kbuttongroup.cpp
    widgets/kcapacitybar.cpp
    widgets/kcmodule.cpp
    widgets/kcombobox.cpp
    widgets/kcompletionbox.cpp
    widgets/kcalendarwidget.cpp
    widgets/keditlistwidget.cpp
    widgets/khbox.cpp
    widgets/khelpmenu.cpp
    widgets/khistorycombobox.cpp
    widgets/kkeysequencewidget.cpp
    widgets/kled.cpp
    widgets/klineedit.cpp
    widgets/kmainwindow.cpp
    widgets/kmenu.cpp
    widgets/kmessagewidget.cpp
    widgets/kmultitabbar.cpp
    widgets/knuminput.cpp
    widgets/kpixmapwidget.cpp
    widgets/kpushbutton.cpp
    widgets/kratingpainter.cpp
    widgets/kratingwidget.cpp
    widgets/krestrictedline.cpp
    widgets/krichtextedit.cpp
    widgets/krichtextwidget.cpp
    widgets/kruler.cpp
    widgets/kselector.cpp
    widgets/kseparator.cpp
    widgets/kshortcutwidget.cpp
    widgets/ksqueezedtextlabel.cpp
    widgets/kstatusbar.cpp
    widgets/ktabbar.cpp
    widgets/ktabwidget.cpp
    widgets/ktextedit.cpp
    widgets/ktimeedit.cpp
    widgets/ktimezonewidget.cpp
    widgets/ktitlewidget.cpp
    widgets/ktoolbar.cpp
    widgets/kurllabel.cpp
    widgets/kvbox.cpp
    widgets/kxyselector.cpp
    widgets/kdeuiwidgetsproxystyle.cpp
    widgets/nestedlisthelper.cpp
    xmlgui/kxmlguibuilder.cpp
    xmlgui/kxmlguiwindow.cpp
    xmlgui/kmainwindowiface.cpp
    xmlgui/kmenumenuhandler_p.cpp
    xmlgui/ktoolbarhandler.cpp
    xmlgui/kxmlguiclient.cpp
    xmlgui/kxmlguifactory.cpp
    xmlgui/kxmlguifactory_p.cpp
    xmlgui/kxmlguiversionhandler.cpp
    windowmanagement/kwindowsystem_x11.cpp
    windowmanagement/kwindowinfo_x11.cpp
    windowmanagement/netwm.cpp
)

qt4_add_dbus_interface(kdeui_LIB_SRCS
    notifications/org.kde.StatusNotifierWatcher.xml
    statusnotifierwatcher_interface
)

qt4_add_dbus_adaptor(kdeui_LIB_SRCS
    notifications/org.kde.StatusNotifierItem.xml
    notifications/kstatusnotifieritemdbus_p.h
    KStatusNotifierItemDBus
)

if (X11_Xkb_FOUND AND X11_Xkbfile_FOUND)
    include_directories (
        ${X11_Xkb_INCLUDE_PATH}
        ${X11_Xlib_INCLUDE_PATH}
    )
    set(KDEUI_EXTRA_LIBS
        ${KDEUI_EXTRA_LIBS}
        ${X11_LIBRARIES}
        ${X11_Xkbfile_LIB}
    )
endif()

qt4_add_dbus_interfaces(kdeui_LIB_SRCS jobs/org.kde.JobViewServer.xml )

qt4_add_dbus_interface(kdeui_LIB_SRCS
    jobs/org.kde.JobView.xml
    jobviewiface
)

set(kglobalaccel_xml shortcuts/org.kde.KGlobalAccel.xml)
set_source_files_properties(${kglobalaccel_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt4_add_dbus_interface(kdeui_LIB_SRCS ${kglobalaccel_xml} kglobalaccel_interface )
install(FILES ${kglobalaccel_xml} DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR})

set(kglobalaccel_component_xml shortcuts/org.kde.kglobalaccel.Component.xml)
set_source_files_properties(${kglobalaccel_component_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt4_add_dbus_interface(kdeui_LIB_SRCS ${kglobalaccel_component_xml} kglobalaccel_component_interface )
install(FILES ${kglobalaccel_component_xml} DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR})

set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/kdeui/colors/kcolordialog.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/kernel/kapplication.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/kernel/kglobalsettings.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/kernel/kuniqueapplication.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/kernel/ktoolinvocation.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/kernel/ktoolinvocation_x11.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/notifications/knotificationrestrictions.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/util/kcrash.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/util/kcursor.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/util/kkeyserver_x11.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/util/kpixmap.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/util/kselectionowner.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/util/kxerrorhandler.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/util/kxmessages.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/windowmanagement/kwindowinfo_x11.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/windowmanagement/kwindowsystem_x11.cpp
    ${CMAKE_SOURCE_DIR}/kdeui/windowmanagement/netwm.cpp
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE
)

add_library(kdeui SHARED ${kdeui_LIB_SRCS})

target_link_libraries(kdeui PRIVATE
    ${X11_LIBRARIES}
    ${ENCHANT_LIBRARIES}
    ${KDEUI_EXTRA_LIBS}
)

target_link_libraries(kdeui PUBLIC
    ${QT_QTGUI_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    kdecore
)

if(X11_XTest_FOUND)
   target_link_libraries(kdeui PRIVATE ${X11_XTest_LIB})
endif()

if(X11_Xcursor_FOUND)
   target_link_libraries(kdeui PRIVATE ${X11_Xcursor_LIB})
endif()

if(X11_Xfixes_FOUND)
   target_link_libraries(kdeui PRIVATE ${X11_Xfixes_LIB})
endif()

set_target_properties(kdeui PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

########### install files ###############

generate_export_header(kdeui)

install(
    TARGETS kdeui
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

install(
    FILES
    dialogs/aboutkde.png
    dialogs/ktip-bulb.png
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/kdeui/pics
)

install(
    FILES xmlgui/ui_standards.rc
    DESTINATION ${KDE4_CONFIG_INSTALL_DIR}/ui
)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kdeui_export.h
    actions/kactioncollection.h
    actions/kactioncategory.h
    actions/kaction.h
    actions/kactionmenu.h
    actions/kdualaction.h
    actions/kcodecaction.h
    actions/kfontaction.h
    actions/kfontsizeaction.h
    actions/kpastetextaction.h
    actions/krecentfilesaction.h
    actions/kselectaction.h
    actions/kstandardaction.h
    actions/ktoggleaction.h
    actions/ktoggletoolbaraction.h
    actions/ktogglefullscreenaction.h
    actions/ktoolbarlabelaction.h
    actions/ktoolbarpopupaction.h
    actions/ktoolbarspaceraction.h
    colors/kcolorutils.h
    colors/kcolorscheme.h
    colors/kcolorcollection.h
    colors/kcolorbutton.h
    colors/kcolorchoosermode.h
    colors/kcolorcombo.h
    colors/kcolordialog.h
    colors/kcolormimedata.h
    colors/kcolorvalueselector.h
    colors/khuesaturationselect.h
    config/kconfigskeleton.h
    dialogs/kaboutapplicationdialog.h
    dialogs/kconfigdialog.h
    dialogs/kconfigdialogmanager.h
    dialogs/kdeprintdialog.h
    dialogs/kdialog.h
    dialogs/kedittoolbar.h
    dialogs/kinputdialog.h
    dialogs/kmessagebox.h
    dialogs/kmessageboxmessagehandler.h
    dialogs/knewpassworddialog.h
    dialogs/kpassworddialog.h
    dialogs/kprogressdialog.h
    dialogs/ksplashscreen.h
    dialogs/kshortcutsdialog.h
    dialogs/kshortcutseditor.h
    dialogs/ktimerdialog.h
    dialogs/ktip.h
    findreplace/kfind.h
    findreplace/kreplace.h
    findreplace/kfinddialog.h
    findreplace/kreplacedialog.h
    fonts/kfontchooser.h
    fonts/kfontcombobox.h
    fonts/kfontdialog.h
    fonts/kfontrequester.h
    fonts/kfontutils.h
    icons/kiconeffect.h
    icons/kicon.h
    icons/kiconloader.h
    icons/kicontheme.h
    itemviews/klinkitemselectionmodel.h
    itemviews/krecursivefilterproxymodel.h
    itemviews/klistwidget.h
    itemviews/klistwidgetsearchline.h
    itemviews/ktreewidgetsearchline.h
    itemviews/kfilterproxysearchline.h
    itemviews/kextendableitemdelegate.h
    itemviews/kcategorizedsortfilterproxymodel.h
    itemviews/kcategorizedview.h
    itemviews/kcategorydrawer.h
    itemviews/kmodelindexproxymapper.h
    itemviews/kwidgetitemdelegate.h
    # itemviews/kreparentingproxymodel.h
    jobs/kdialogjobuidelegate.h
    jobs/kabstractwidgetjobtracker.h
    jobs/kwidgetjobtracker.h
    jobs/kstatusbarjobtracker.h
    jobs/kuiserverjobtracker.h
    jobs/kdynamicjobtracker.h
    kernel/kapplication.h
    kernel/kuniqueapplication.h
    kernel/ksessionmanager.h
    kernel/kstyle.h
    kernel/kstartupinfo.h
    kernel/kglobalsettings.h
    kernel/ksystemeventfilter.h
    kernel/ktoolinvocation.h
    notifications/kstatusnotifieritem.h
    notifications/ksystemtrayicon.h
    notifications/knotification.h
    notifications/knotificationconfigwidget.h
    notifications/knotificationrestrictions.h
    notifications/kdbusmenuexporter.h
    notifications/kdbusmenuimporter.h
    paged/kpagedialog.h
    paged/kpageview.h
    paged/kpagewidget.h
    paged/kpagewidgetmodel.h
    shortcuts/kshortcut.h
    shortcuts/kstandardshortcut.h
    shortcuts/kglobalaccel.h
    shortcuts/kglobalshortcutinfo.h
    shortcuts/kglobalshortcutinfo_p.h
    shortcuts/kacceleratormanager.h
    spell/kspeller.h
    spell/kspellhighlighter.h
    spell/kspelldictionarycombobox.h
    spell/kspellconfigwidget.h
    util/kaccelgen.h
    util/kcompletion.h
    util/kcrash.h
    util/kcursor.h
    util/kguiitem.h
    util/kkeyserver.h
    util/kkeyboardlayout.h
    util/kselectionowner.h
    util/knumvalidator.h
    util/kpassivepopup.h
    util/kpassivepopupmessagehandler.h
    util/kstandardguiitem.h
    util/kxerrorhandler.h
    util/kxmessages.h
    util/kundostack.h
    util/kpixmap.h
    util/kpixmapsequence.h
    util/kpixmapsequenceoverlaypainter.h
    util/kpixmapsequencewidget.h
    util/kimageio.h
    widgets/kactionselector.h
    widgets/kcalendarwidget.h
    widgets/kcapacitybar.h
    widgets/kcmodule.h
    widgets/kcombobox.h
    widgets/kcompletionbox.h
    widgets/kbuttongroup.h
    widgets/kdialogbuttonbox.h
    widgets/keditlistwidget.h
    widgets/khbox.h
    widgets/khelpmenu.h
    widgets/khistorycombobox.h
    widgets/kkeysequencewidget.h
    widgets/kled.h
    widgets/klineedit.h
    widgets/kmainwindow.h
    widgets/kmenu.h
    widgets/kmessagewidget.h
    widgets/kmultitabbar.h
    widgets/knuminput.h
    widgets/kpixmapwidget.h
    widgets/kpushbutton.h
    widgets/kratingpainter.h
    widgets/kratingwidget.h
    widgets/krestrictedline.h
    widgets/krichtextedit.h
    widgets/krichtextwidget.h
    widgets/kseparator.h
    widgets/ksqueezedtextlabel.h
    widgets/ktextedit.h
    widgets/ktimeedit.h
    widgets/ktimezonewidget.h
    widgets/ktitlewidget.h
    widgets/ktabbar.h
    widgets/ktabwidget.h
    widgets/kruler.h
    widgets/kselector.h
    widgets/kshortcutwidget.h
    widgets/kstatusbar.h
    widgets/ktoolbar.h
    widgets/kurllabel.h
    widgets/kvbox.h
    widgets/kxyselector.h
    windowmanagement/kwindowsystem.h
    windowmanagement/kwindowinfo.h
    windowmanagement/netwm_def.h
    windowmanagement/netwm.h
    xmlgui/kxmlguiwindow.h
    xmlgui/kxmlguiclient.h
    xmlgui/kxmlguibuilder.h
    xmlgui/kxmlguifactory.h
    util/fixx11h.h
    util/kkeyserver_x11.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}
)

# KDE 5 remove this
install(
    FILES
    widgets/kratingpainter.h
    widgets/kratingwidget.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/nepomuk
)

install(
    PROGRAMS preparetips
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)

install(
    FILES
    colors/40.colors
    colors/Oxygen.colors
    colors/Web.colors
    colors/Royal.colors
    colors/Rainbow.colors
    DESTINATION ${KDE4_CONFIG_INSTALL_DIR}/colors
)

install(
    FILES
    jobs/org.kde.JobViewServer.xml
    jobs/org.kde.JobView.xml
    notifications/org.kde.StatusNotifierItem.xml
    notifications/org.kde.StatusNotifierWatcher.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)
