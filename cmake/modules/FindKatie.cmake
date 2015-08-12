# - Try to find the Katie Toolkit
#
# Once done this will define:
#
#  KATIE_FOUND - system has libmng
#  KATIE_INCLUDES - the libmng include directory
#  KATIE_LIBRARIES - The libraries needed to use libmng
#
#  KATIE_<COMPONENT>_INCLUDES
#  KATIE_<COMPONENT>_LIBRARIES
#
#  KATIE_MKSPECS_DIR
#  KATIE_MOC_EXECUTABLE
#  KATIE_UIC_EXECUTABLE
#  KATIE_RCC_EXECUTABLE
#  KATIE_QDBUSXML2CPP_EXECUTABLE
#  KATIE_QDBUSCPP2XML_EXECUTABLE
#  KATIE_QHELPGENERATOR_EXECUTABLE
#  KATIE_QCOLLECTIONGENERATOR_EXECUTABLE
#  KATIE_LUPDATE_EXECUTABLE
#  KATIE_LRELEASE_EXECUTABLE
#  KATIE_LCONVERT_EXECUTABLE
#
# In addition the following macros will be defined:
#
#  KATIE_RESOURCES( < source.cpp | header.h | resource.qrc | userinterface.ui > ...)
#  KATIE_DBUS_INTERFACE()
#  KATIE_DBUS_ADAPTOR()
#  KATIE_TRANSLATION()
#
# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.

if(KATIE_INCLUDES AND KATIE_LIBRARIES)
  set(KATIE_FIND_QUIETLY TRUE)
endif(KATIE_INCLUDES AND KATIE_LIBRARIES)

if(Katie_FIND_COMPONENTS)
    # for compat with Qt
    foreach(comp ${Katie_FIND_COMPONENTS})
        string(REPLACE REGEX "^Qt" "" modcomp ${comp})
        set(KATIECOMPONENTS ${modcomp})
    endforeach()
else()
    # TODO: add Multimedia once it builds
    set(KATIECOMPONENTS Core Gui Network OpenGL Sql Svg Test DBus Xml XmlPatterns Script ScriptTools WebKit Declarative Help UiTools Designer)
endif()
# TODO: designer, linguist?
set(KATIETOOLS moc uic rcc qdbusxml2cpp qdbuscpp2xml qhelpgenerator qcollectiongenerator lupdate lrelease lconvert)

set(KATIE_FOUND TRUE)
set(KATIE_INCLUDES ${CMAKE_BINARY_DIR}/_generated_)
set(KATIE_LIBRARIES)

find_path(KATIE_MKSPECS_DIR
    NAMES
    mkspecs
    PATH_SUFFIXES qt4
    HINTS
    /share
    /usr/share
    /usr/local/share
    $ENV{QTDIR}/share
)

foreach(tool ${KATIETOOLS})
    string(TOUPPER ${tool} uppertool)
    find_program(KATIE_${uppertool}_EXECUTABLE
        NAMES
        ${tool} ${tool}-qt4 ${tool}-katie
        HINTS
        /bin
        /usr/bin
        /usr/local/bin
        $ENV{QTDIR}/bin
    )
endforeach()

foreach(component ${KATIECOMPONENTS})
    string(TOUPPER KATIE_${component} uppercomp)
    set(component Qt${component})

    if(NOT WIN32)
        # use pkg-config to get the directories and then use these values
        # in the FIND_PATH() and FIND_LIBRARY() calls
        find_package(PkgConfig)
        pkg_check_modules(PC_${uppercomp} QUIET ${component})
    endif(NOT WIN32)

    find_path(FIND_${uppercomp}_INCLUDES
        NAMES
        ${component}
        PATH_SUFFIXES ${component}
        HINTS
        /include
        /usr/include
        /usr/local/include
        $ENV{QTDIR}/include
        ${PC_${uppercomp}_INCLUDEDIR}
        ${INCLUDE_INSTALL_DIR}
    )

    find_library(FIND_${uppercomp}_LIBRARIES
        ${component}
        HINTS
        /lib
        /usr/lib
        /usr/local/lib
        $ENV{QTDIR}/lib
        ${PC_${uppercomp}_LIBDIR}
        ${LIB_INSTALL_DIR}
    )

    set(COMPONENT_INCLUDES ${FIND_${uppercomp}_INCLUDES})
    set(COMPONENT_LIBRARIES ${FIND_${uppercomp}_LIBRARIES} ${PC_${uppercomp}_LIBRARIES})
    set(COMPONENT_VERSION ${PC_${uppercomp}_VERSION})
    if(NOT COMPONENT_VERSION)
        set(COMPONENT_VERSION "unknown")
    endif()
    if(NOT "${COMPONENT_INCLUDES}" STREQUAL "${uppercomp}_INCLUDES-NOTFOUND"
        AND NOT "${COMPONENT_LIBRARIES}" STREQUAL "${uppercomp}_LIBRARIES-NOTFOUND")
        message(STATUS "Found ${component}, version ${COMPONENT_VERSION}")
        set(${uppercomp}_FOUND TRUE)
        get_filename_component(parentinclude ${COMPONENT_INCLUDES} DIRECTORY) 
        set(KATIE_INCLUDES ${KATIE_INCLUDES} ${COMPONENT_INCLUDES} ${parentinclude})
        set(KATIE_LIBRARIES ${KATIE_LIBRARIES} ${COMPONENT_LIBRARIES})
        set(${uppercomp}_INCLUDES ${COMPONENT_INCLUDES})
        set(${uppercomp}_LIBRARIES ${COMPONENT_LIBRARIES})
    else()
        message(STATUS "Could not find: ${component}")
        set(${uppercomp}_FOUND FALSE)
        set(KATIE_FOUND FALSE)
    endif()
endforeach()

include(KatieMacros)

if(KATIE_MKSPECS_DIR)
    include(${KATIE_MKSPECS_DIR}/mkspecs/mkspecs.cmake)
endif()

if(${KATIE_COMPAT} AND KATIE_FOUND)
    set(Qt4_FOUND TRUE)
    set(QT_FOUND TRUE)
    set(QT4_FOUND TRUE)
    set(QT_VERSION_MAJOR ${KATIE_MAJOR})
    set(QT_VERSION_MINOR ${KATIE_MINOR})
    set(QT_VERSION_PATCH ${KATIE_MICRO})
    set(QT_VERSION ${KATIE_VERSION})
    set(QT_INCLUDES ${KATIE_INCLUDES})
    set(QT_INCLUDE_DIR ${KATIE_INCLUDES})
    set(QT_LIBRARIES ${KATIE_LIBRARIES})
    set(QT_USE_FILE ${KATIE_MKSPECS_DIR}/mkspecs/mkspecs.cmake)

    set(QT_MOC_EXECUTABLE ${KATIE_MOC_EXECUTABLE})
    set(QT_UIC_EXECUTABLE ${KATIE_UIC_EXECUTABLE})
    set(QT_RCC_EXECUTABLE ${KATIE_RCC_EXECUTABLE})
    set(QT_DBUSXML2CPP_EXECUTABLE ${KATIE_QDBUSXML2CPP_EXECUTABLE})
    set(QT_DBUSCPP2XML_EXECUTABLE ${KATIE_QDBUSCPP2XML_EXECUTABLE})
    set(QT_LUPDATE_EXECUTABLE ${KATIE_LUPDATE_EXECUTABLE})
    set(QT_LRELEASE_EXECUTABLE ${KATIE_LRELEASE_EXECUTABLE})
    set(QT_MKSPECS_DIR ${KATIE_MKSPECS_DIR})

    if(NOT "${KATIE_FIND_QUIETLY}")
        foreach(tool ${KATIETOOLS})
            string(TOUPPER ${tool} uppertool)
            add_executable(Qt4::${tool} IMPORTED)
            set_property(TARGET Qt4::${tool} PROPERTY IMPORTED_LOCATION ${KATIE_${uppertool}_EXECUTABLE})
        endforeach()

        foreach(component ${KATIECOMPONENTS})
            string(TOUPPER ${component} uppercomp)
            add_library(Qt4::Qt${component} ${KATIE_TYPE} IMPORTED)
            set_property(TARGET Qt4::Qt${component} PROPERTY IMPORTED_LOCATION ${FIND_KATIE_${uppercomp}_LIBRARIES})
        endforeach()
    endif()

    # bad assumption
    if(UNIX)
        set(Q_WS_X11 TRUE)
        find_package(X11 REQUIRED)
    endif()

    foreach(component ${KATIECOMPONENTS})
        string(TOUPPER ${component} uppercomp)
        set(QT_QT${uppercomp}_FOUND "${KATIE_${uppercomp}_FOUND}")
        set(QT_QT${uppercomp}_LIBRARY "${KATIE_${uppercomp}_LIBRARIES}")
    endforeach()
    include(Qt4Macros)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Katie DEFAULT_MSG KATIE_MKSPECS_DIR KATIE_INCLUDES KATIE_LIBRARIES)

mark_as_advanced(KATIE_MKSPECS_DIR KATIE_INCLUDES KATIE_LIBRARIES)
