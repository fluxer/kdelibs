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
#  KATIE_DBUSXML2CPP_EXECUTABLE
#  KATIE_DBUSCPP2XML_EXECUTABLE
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
    set(KATIECOMPONENTS ${Katie_FIND_COMPONENTS})
else()
    set(KATIECOMPONENTS Core Gui Network Multimedia OpenGL Sql Svg Test DBus Xml XmlPatterns Script ScriptTools WebKit Declarative)
endif()

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

find_program(KATIE_MOC_EXECUTABLE
    NAMES
    moc moc-qt4
    HINTS
    /bin
    /usr/bin
    /usr/local/bin
    $ENV{QTDIR}/bin
)

find_program(KATIE_UIC_EXECUTABLE
    NAMES
    uic uic-qt4
    HINTS
    /bin
    /usr/bin
    /usr/local/bin
    $ENV{QTDIR}/bin
)

find_program(KATIE_RCC_EXECUTABLE
    NAMES
    rcc rcc-qt4
    HINTS
    /bin
    /usr/bin
    /usr/local/bin
    $ENV{QTDIR}/bin
)

find_program(KATIE_DBUSXML2CPP_EXECUTABLE
    NAMES
    qdbusxml2cpp qdbusxml2cpp-qt4
    HINTS
    /bin
    /usr/bin
    /usr/local/bin
    $ENV{QTDIR}/bin
)

find_program(KATIE_DBUSCPP2XML_EXECUTABLE
    NAMES
    qdbuscpp2xml qdbuscpp2xml-qt4
    HINTS
    /bin
    /usr/bin
    /usr/local/bin
    $ENV{QTDIR}/bin
)

foreach(component ${KATIECOMPONENTS})
    string(TOUPPER KATIE_${component} uppercomp)
    set(component Qt${component})

    if(NOT WIN32)
        # use pkg-config to get the directories and then use these values
        # in the FIND_PATH() and FIND_LIBRARY() calls
        find_package(PkgConfig)
        pkg_check_modules(PC_${uppercomp} QUIET ${component})
    endif(NOT WIN32)

    find_path(${uppercomp}_INCLUDES
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

    find_library(${uppercomp}_LIBRARIES
        ${component}
        HINTS
        /lib
        /usr/lib
        /usr/local/lib
        $ENV{QTDIR}/lib
        ${PC_${uppercomp}_LIBDIR}
        ${LIB_INSTALL_DIR}
    )

    set(COMPONENT_INCLUDES ${${uppercomp}_INCLUDES})
    set(COMPONENT_LIBRARIES ${${uppercomp}_LIBRARIES} ${PC_${uppercomp}_LIBRARIES})
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
    set(QT_VERSION_MINOR.${KATIE_MINOR})
    set(QT_VERSION_PATCH ${KATIE_MICRO})
    set(QT_VERSION ${KATIE_VERSION})
    set(QT_INCLUDES ${KATIE_INCLUDES})
    set(QT_INCLUDE_DIR ${KATIE_INCLUDES})

    set(QT_MOC_EXECUTABLE ${KATIE_MOC_EXECUTABLE})
    set(QT_UIC_EXECUTABLE ${KATIE_UIC_EXECUTABLE})
    set(QT_RCC_EXECUTABLE ${KATIE_RCC_EXECUTABLE})
    set(QT_DBUSXML2CPP_EXECUTABLE ${KATIE_DBUSXML2CPP_EXECUTABLE})
    set(QT_DBUSCPP2XML_EXECUTABLE ${KATIE_DBUSCPP2XML_EXECUTABLE})
    set(QT_MKSPECS_DIR ${KATIE_MKSPECS_DIR})

    add_executable(Qt4::moc IMPORTED)
    set_property(TARGET Qt4::moc PROPERTY IMPORTED_LOCATION ${KATIE_MOC_EXECUTABLE})
    add_executable(Qt4::uic IMPORTED)
    set_property(TARGET Qt4::uic PROPERTY IMPORTED_LOCATION ${KATIE_UIC_EXECUTABLE})
    add_executable(Qt4::rcc IMPORTED)
    set_property(TARGET Qt4::rcc PROPERTY IMPORTED_LOCATION ${KATIE_RCC_EXECUTABLE})

    set(QT_QTCORE_FOUND "${KATIE_CORE_FOUND}")
    set(QT_QTCORE_LIBRARY "${KATIE_CORE_LIBRARIES}")
    set(QT_QTGUI_FOUND "${KATIE_GUI_FOUND}")
    set(QT_QTGUI_LIBRARY "${KATIE_GUI_LIBRARIES}")
    set(QT_QTNETWORK_FOUND "${KATIE_NETWORK_FOUND}")
    set(QT_QTNETWORK_LIBRARY "${KATIE_NETWORK_LIBRARIES}")
    set(QT_QTMULTIMEDIA_FOUND "${KATIE_MULTIMEDIA_FOUND}")
    set(QT_QTMULTIMEDIA_LIBRARY "${KATIE_MULTIMEDIA_LIBRARIES}")
    set(QT_QTOPENGL_FOUND "${KATIE_OPENGL_FOUND}")
    set(QT_QTOPENGL_LIBRARY "${KATIE_OPENGL_LIBRARIES}")
    set(QT_QTSQL_FOUND "${KATIE_SQL_FOUND}")
    set(QT_QTSQL_LIBRARY "${KATIE_SQL_LIBRARIES}")
    set(QT_QTSVG_FOUND "${KATIE_SVG_FOUND}")
    set(QT_QTSVG_LIBRARY "${KATIE_SVG_LIBRARIES}")
    set(QT_QTTEST_FOUND "${KATIE_TEST_FOUND}")
    set(QT_QTTEST_LIBRARY "${KATIE_TEST_LIBRARIES}")
    set(QT_QTDBUS_LIBRARY "${KATIE_DBUS_LIBRARIES}")
    set(QT_QTDBUS_FOUND "${KATIE_DBUS_FOUND}")
    set(QT_QTXML_FOUND "${KATIE_XML_FOUND}")
    set(QT_QTXML_LIBRARY "${KATIE_XML_LIBRARIES}")
    set(QT_QTXMLPATTERNS_FOUND "${KATIE_XMLPATTERNS_FOUND}")
    set(QT_QTXMLPATTERNS_LIBRARY "${KATIE_XMLPATTERNS_LIBRARIES}")
    set(QT_QTSCRIPT_FOUND "${KATIE_SCRIPT_FOUND}")
    set(QT_QTSCRIPT_LIBRARY "${KATIE_SCRIPT_LIBRARIES}")
    set(QT_QTSCRIPTTOOLS_FOUND "${KATIE_SCRIPTTOOLS_FOUND}")
    set(QT_QTSCRIPTTOOLS_LIBRARY "${KATIE_SCRIPTTOOLS_LIBRARIES}")
    set(QT_QTWEBKIT_FOUND "${KATIE_WEBKIT_FOUND}")
    set(QT_QTWEBKIT_LIBRARY "${KATIE_WEBKIT_LIBRARIES}")
    set(QT_QTDECLARATIVE_FOUND "${KATIE_DECLARATIVE_FOUND}")
    set(QT_QTDECLARATIVE_LIBRARY "${KATIE_DECLARATIVE_LIBRARIES}")
    include(Qt4Macros)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Katie DEFAULT_MSG KATIE_MKSPECS_DIR KATIE_INCLUDES KATIE_LIBRARIES)

mark_as_advanced(KATIE_MKSPECS_DIR KATIE_INCLUDES KATIE_LIBRARIES)
