if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_SOLID TRUE)
    set(PLASMA_NO_KIO TRUE)
    set(PLASMA_NO_KUTILS TRUE)
    set(PLASMA_NO_GLOBAL_SHORTCUTS TRUE)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kdeclarative
    ${CMAKE_BINARY_DIR}/kdeclarative
    ${CMAKE_SOURCE_DIR}/plasma/extenders
    # for the export headers
    ${CMAKE_BINARY_DIR}/kutils
)

if(NOT PLASMA_NO_KIO)
   include_directories(${KDE4_KIO_INCLUDES})
   set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} kio)
endif()

if(NOT PLASMA_NO_SOLID)
    include_directories(${CMAKE_BINARY_DIR}/solid/)
    include_directories(${CMAKE_SOURCE_DIR}/solid/)
    set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} solid)
endif()

if(NOT PLASMA_NO_KUTILS)
    include_directories(${CMAKE_SOURCE_DIR}/kutils)
    set(PLASMA_EXTRA_LIBS ${PLASMA_EXTRA_LIBS} kcmutils)
endif()

configure_file(
    config-plasma.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-plasma.h
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1209)

########### next target ###############

set(plasma_LIB_SRCS
    packagemetadata.cpp
    packagestructure.cpp
    package.cpp
    abstractrunner.cpp
    abstracttoolbox.cpp
    animator.cpp
    animations/animation.cpp
    animations/easinganimation.cpp
    animations/fade.cpp
    animations/pixmaptransition.cpp
    animations/pulser.cpp
    animations/slide.cpp
    animations/geometry.cpp
    animations/widgetsnapshot.cpp
    animations/zoom.cpp
    applet.cpp
    configloader.cpp
    containment.cpp
    containmentactions.cpp
    containmentactionspluginsconfig.cpp
    corona.cpp
    datacontainer.cpp
    dataengine.cpp
    dataenginemanager.cpp
    delegate.cpp
    dialog.cpp
    dialogshadows.cpp
    extenders/extender.cpp
    extenders/extendergroup.cpp
    extenders/extenderitem.cpp
    pluginloader.cpp
    paintutils.cpp
    framesvg.cpp
    plasma.cpp
    popupapplet.cpp
    private/applethandle.cpp
    private/associatedapplicationmanager.cpp
    private/datacontainer_p.cpp
    private/dataenginebindings.cpp
    private/dataengineconsumer.cpp
    private/extenderapplet.cpp
    private/extenderitemmimedata.cpp
    private/nativetabbar.cpp
    private/packages.cpp
    private/runnerjobs.cpp
    private/style.cpp
    private/themedwidgetinterface.cpp
    private/tooltip.cpp
    private/windowpreview.cpp
    private/effects/halopainter.cpp
    querymatch.cpp
    runnercontext.cpp
    runnermanager.cpp
    runnersyntax.cpp
    scripting/appletscript.cpp
    scripting/scriptengine.cpp
    service.cpp
    servicejob.cpp
    svg.cpp
    theme.cpp
    tooltipcontent.cpp
    tooltipmanager.cpp
    view.cpp
    wallpaper.cpp
    windoweffects.cpp
    widgets/checkbox.cpp
    widgets/combobox.cpp
    widgets/declarativewidget.cpp
    widgets/flashinglabel.cpp
    widgets/frame.cpp
    widgets/groupbox.cpp
    widgets/iconwidget.cpp
    widgets/itembackground.cpp
    widgets/label.cpp
    widgets/lineedit.cpp
    widgets/meter.cpp
    widgets/pushbutton.cpp
    widgets/radiobutton.cpp
    widgets/scrollbar.cpp
    widgets/signalplotter.cpp
    widgets/slider.cpp
    widgets/spinbox.cpp
    widgets/toolbutton.cpp
    widgets/busywidget.cpp
    widgets/scrollwidget.cpp
    widgets/separator.cpp
    widgets/svgwidget.cpp
    widgets/tabbar.cpp
    widgets/textbrowser.cpp
    widgets/treeview.cpp
    widgets/textedit.cpp
    widgets/calendarwidget.cpp
)

set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/plasma/dialogshadows.cpp
    ${CMAKE_SOURCE_DIR}/plasma/windoweffects.cpp
    ${CMAKE_SOURCE_DIR}/plasma/widgets/label.cpp
    ${CMAKE_SOURCE_DIR}/plasma/widgets/pushbutton.cpp
    ${CMAKE_SOURCE_DIR}/plasma/widgets/scrollwidget.cpp
    ${CMAKE_SOURCE_DIR}/plasma/view.cpp
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE
)

add_library(plasma SHARED ${plasma_LIB_SRCS})

target_link_libraries(plasma PUBLIC
    ${QT_QTUITOOLS_LIBRARY}
    ${QT_QTSCRIPT_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTSVG_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${PLASMA_EXTRA_LIBS}
    kdecore
    kdeui
    kdeclarative
)

target_link_libraries(plasma PRIVATE ${X11_LIBRARIES})

set_target_properties(plasma PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS plasma
    EXPORT kdelibsTargets
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

########### install files ###############

generate_export_header(plasma)

install(
    FILES
    packagemetadata.h
    packagestructure.h
    package.h
    abstractrunner.h
    abstracttoolbox.h
    animations/animation.h
    animator.h
    applet.h
    configloader.h
    containment.h
    containmentactions.h
    containmentactionspluginsconfig.h
    corona.h
    datacontainer.h
    dataengine.h
    dataenginemanager.h
    delegate.h
    dialog.h
    dialogshadows.h
    extenders/extender.h
    extenders/extendergroup.h
    extenders/extenderitem.h
    pluginloader.h
    paintutils.h
    windoweffects.h
    framesvg.h
    plasma.h
    popupapplet.h
    querymatch.h
    runnercontext.h
    runnermanager.h
    runnersyntax.h
    service.h
    servicejob.h
    svg.h
    theme.h
    tooltipcontent.h
    tooltipmanager.h
    view.h
    wallpaper.h
    ${CMAKE_CURRENT_BINARY_DIR}/plasma_export.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/plasma
)

install(
    FILES
    widgets/checkbox.h
    widgets/combobox.h
    widgets/declarativewidget.h
    widgets/flashinglabel.h
    widgets/frame.h
    widgets/groupbox.h
    widgets/iconwidget.h
    widgets/itembackground.h
    widgets/label.h
    widgets/lineedit.h
    widgets/meter.h
    widgets/pushbutton.h
    widgets/toolbutton.h
    widgets/radiobutton.h
    widgets/scrollbar.h
    widgets/signalplotter.h
    widgets/slider.h
    widgets/spinbox.h
    widgets/busywidget.h
    widgets/separator.h
    widgets/svgwidget.h
    widgets/scrollwidget.h
    widgets/tabbar.h
    widgets/textbrowser.h
    widgets/treeview.h
    widgets/textedit.h
    widgets/calendarwidget.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/plasma/widgets
)

install(
    FILES
    scripting/appletscript.h
    scripting/scriptengine.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/plasma/scripting
)

install(
    FILES
    animations/animation.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/plasma/animations
)


install(
    FILES
    data/servicetypes/plasma-applet.desktop
    data/servicetypes/plasma-applet-popupapplet.desktop
    data/servicetypes/plasma-containment.desktop
    data/servicetypes/plasma-containmentactions.desktop
    data/servicetypes/plasma-dataengine.desktop
    data/servicetypes/plasma-runner.desktop
    data/servicetypes/plasma-scriptengine.desktop
    data/servicetypes/plasma-toolbox.desktop
    data/servicetypes/plasma-wallpaper.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)

install(
    FILES
    data/services/plasma-applet-extenderapplet.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)

