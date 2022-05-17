add_subdirectory(ifaces)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_QTDBUS_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
)

########### next target ###############

make_directory(${CMAKE_CURRENT_BINARY_DIR}/backends/fakehw)
make_directory(${CMAKE_CURRENT_BINARY_DIR}/backends/udev)

set(solid_OPTIONAL_LIBS)

set(solid_LIB_SRCS
    networking.cpp
    solidnamespace.cpp
    managerbase.cpp
    device.cpp
    devicemanager.cpp
    deviceinterface.cpp
    genericinterface.cpp
    processor.cpp
    block.cpp
    storagedrive.cpp
    opticaldrive.cpp
    storagevolume.cpp
    opticaldisc.cpp
    storageaccess.cpp
    camera.cpp
    portablemediaplayer.cpp
    networkinterface.cpp
    networkshare.cpp
    acadapter.cpp
    battery.cpp
    button.cpp
    audiointerface.cpp
    predicate.cpp
    predicateparse.cpp
    predicate_lexer.c
    predicate_parser.c
    powermanagement.cpp
    video.cpp
    graphic.cpp

    ifaces/acadapter.cpp
    ifaces/audiointerface.cpp
    ifaces/battery.cpp
    ifaces/block.cpp
    ifaces/button.cpp
    ifaces/camera.cpp
    ifaces/opticaldrive.cpp
    ifaces/device.cpp
    ifaces/deviceinterface.cpp
    ifaces/devicemanager.cpp
    ifaces/genericinterface.cpp
    ifaces/networkinterface.cpp
    ifaces/networkshare.cpp
    ifaces/opticaldisc.cpp
    ifaces/portablemediaplayer.cpp
    ifaces/processor.cpp
    ifaces/storagedrive.cpp
    ifaces/storagevolume.cpp
    ifaces/storageaccess.cpp
    ifaces/video.cpp
    ifaces/graphic.cpp

    backends/shared/rootdevice.cpp
    backends/exports/exportsdevice.cpp
    backends/exports/exportsdeviceinterface.cpp
    backends/exports/exportsmanager.cpp
    backends/exports/exportsnetworkshare.cpp
    backends/exports/exportsstorageaccess.cpp
)

configure_file(
    config-solid.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-solid.h
)

if(ENABLE_TESTING)
    message(STATUS "Building Solid fake backend.")
    set(solid_LIB_SRCS ${solid_LIB_SRCS}
        backends/fakehw/fakeacadapter.cpp
        backends/fakehw/fakeaudiointerface.cpp
        backends/fakehw/fakebattery.cpp
        backends/fakehw/fakeblock.cpp
        backends/fakehw/fakebutton.cpp
        backends/fakehw/fakecamera.cpp
        backends/fakehw/fakecdrom.cpp
        backends/fakehw/fakedevice.cpp
        backends/fakehw/fakedeviceinterface.cpp
        backends/fakehw/fakegenericinterface.cpp
        backends/fakehw/fakemanager.cpp
        backends/fakehw/fakenetworkshare.cpp
        backends/fakehw/fakenetworkinterface.cpp
        backends/fakehw/fakeopticaldisc.cpp
        backends/fakehw/fakeportablemediaplayer.cpp
        backends/fakehw/fakeprocessor.cpp
        backends/fakehw/fakestorage.cpp
        backends/fakehw/fakestorageaccess.cpp
        backends/fakehw/fakevideo.cpp
        backends/fakehw/fakevolume.cpp
        backends/fakehw/fakegraphic.cpp
    )
endif()

if(UDEV_FOUND)
    include_directories(${UDEV_INCLUDES})
    message(STATUS "Building Solid UDev backend.")
    set(solid_LIB_SRCS ${solid_LIB_SRCS}
        backends/udev/udevdevice.cpp
        backends/udev/udevmanager.cpp
        backends/udev/udevdeviceinterface.cpp
        backends/udev/cpuinfo.cpp
        backends/udev/udevprocessor.cpp
        backends/udev/udevcamera.cpp
        backends/udev/udevvideo.cpp
        backends/udev/udevaudiointerface.cpp
        backends/udev/udevnetworkinterface.cpp
        backends/udev/udevaudiointerface_p.cpp
        backends/udev/udevportablemediaplayer.cpp
        backends/udev/udevblock.cpp
        backends/udev/udevbutton.cpp
        backends/udev/udevstorageaccess.cpp
        backends/udev/udevstoragevolume.cpp
        backends/udev/udevstoragedrive.cpp
        backends/udev/udevbattery.cpp
        backends/udev/udevacadapter.cpp
        backends/udev/udevgraphic.cpp
        backends/udev/udevqtclient.cpp
        backends/udev/udevqtdevice.cpp
    )

    if(LIBCDIO_FOUND)
        set(solid_LIB_SRCS ${solid_LIB_SRCS}
            backends/udev/udevopticaldisc.cpp
            backends/udev/udevopticaldrive.cpp
        )
        include_directories(${LIBCDIO_INCLUDES})
        set(solid_OPTIONAL_LIBS ${solid_OPTIONAL_LIBS} ${LIBCDIO_LIBRARIES})
    endif()

    set(UDEV_DETAILED_OUTPUT OFF CACHE BOOL "provide extended output regarding udev events")
    if(UDEV_DETAILED_OUTPUT)
        add_definitions(-DUDEV_DETAILED_OUTPUT)
    endif()

    # check for media-player-info (runtime-only optional dependency)
    set(XDG_DATA_DIRS_ENV $ENV{XDG_DATA_DIRS})  # if(ENV{..}) does not work for me
    if(XDG_DATA_DIRS_ENV)
        find_path(MEDIAPLAYERINFO_PATH sony_psp.mpi
            PATHS ENV XDG_DATA_DIRS
            PATH_SUFFIXES "media-player-info"
            NO_DEFAULT_PATH
        )
    else(XDG_DATA_DIRS_ENV)
        set(XDG_DATA_DIRS "/usr/share")
        message(STATUS "Warning: environment variable XDG_DATA_DIRS not set, falling back to ${XDG_DATA_DIRS}")
        find_path(MEDIAPLAYERINFO_PATH sony_psp.mpi
            PATHS "${XDG_DATA_DIRS}"
            PATH_SUFFIXES "media-player-info"
            NO_DEFAULT_PATH
        )
    endif(XDG_DATA_DIRS_ENV)

    add_feature_info("media-player-info" MEDIAPLAYERINFO_PATH
                    "Enables identification and querying of portable media players.
                    Runtime-only dependency of the udev solid backend.
                    Support for m-p-i is included even if not found during build"
                    )

    set(solid_OPTIONAL_LIBS ${solid_OPTIONAL_LIBS} ${KDE4_KDECORE_LIBS} ${QT_QTDBUS_LIBRARY} ${UDEV_LIBRARIES})
endif(UDEV_FOUND)

if(DEVINFO_FOUND)
    include_directories(${DEVINFO_INCLUDES})
    message(STATUS "Building Solid Devinfo backend.")
    set(solid_LIB_SRCS ${solid_LIB_SRCS}
        backends/devinfo/devinfodevice.cpp
        backends/devinfo/devinfomanager.cpp
        backends/devinfo/devinfodeviceinterface.cpp
        backends/devinfo/devinfoprocessor.cpp
        backends/devinfo/devinfonetworkinterface.cpp
        backends/devinfo/devinfographic.cpp
        backends/devinfo/devdqt.cpp
    )

    set(solid_OPTIONAL_LIBS ${solid_OPTIONAL_LIBS} ${DEVINFO_LIBRARIES})
endif()

qt4_add_dbus_interface(solid_LIB_SRCS
    org.kde.Solid.Networking.Client.xml
    org_kde_solid_networking_client
)

add_library(solid ${LIBRARY_TYPE} ${solid_LIB_SRCS})

target_link_libraries(solid PRIVATE
    ${solid_OPTIONAL_LIBS}
)

target_link_libraries(solid PUBLIC
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

set_target_properties(solid PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(
    TARGETS solid
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(
    FILES
    org.kde.Solid.Networking.Client.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)

########### install files ###############

generate_export_header(solid)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/solid_export.h
    solidnamespace.h
    device.h
    devicenotifier.h
    deviceinterface.h
    genericinterface.h
    processor.h
    block.h
    storageaccess.h
    storagedrive.h
    opticaldrive.h
    storagevolume.h
    opticaldisc.h
    camera.h
    portablemediaplayer.h
    networkinterface.h
    acadapter.h
    battery.h
    button.h
    audiointerface.h
    predicate.h
    powermanagement.h
    networking.h
    video.h 
    networkshare.h
    graphic.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/solid
    COMPONENT Devel
)
