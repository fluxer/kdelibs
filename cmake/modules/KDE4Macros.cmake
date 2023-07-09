# this file contains the following macros (or functions):
# KDE4_INSTALL_ICONS
# KDE4_ADD_KCFG_FILES
# KDE4_ADD_PLUGIN
# KDE4_ADD_TEST
# KDE4_ADD_WIDGET
# KDE4_INSTALL_AUTH_HELPER_FILES
# KDE4_ADD_DBUS_SERVICE
# KDE4_BOOL_TO_01
# KDE4_OPTIONAL_ADD_SUBDIRECTORY
# KDE4_OPTIONAL_FIND_PACKAGE

# Copyright (c) 2006-2010 Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
# Copyright (c) 2015 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# KDE 4 / icon naming specification compatibility
set(_KDE4_ICON_GROUP_mimetypes  "mimetypes")
set(_KDE4_ICON_GROUP_places     "places")
set(_KDE4_ICON_GROUP_devices    "devices")
set(_KDE4_ICON_GROUP_apps       "apps")
set(_KDE4_ICON_GROUP_actions    "actions")
set(_KDE4_ICON_GROUP_categories "categories")
set(_KDE4_ICON_GROUP_status     "status")
set(_KDE4_ICON_GROUP_emblems    "emblems")
set(_KDE4_ICON_GROUP_emotes     "emotes")
set(_KDE4_ICON_GROUP_animations "animations")
set(_KDE4_ICON_GROUP_intl       "intl")

# a "map" of short theme names to the theme directory
set(_KDE4_ICON_THEME_ox "ariya")
set(_KDE4_ICON_THEME_hi "hicolor")

# only used internally by KDE4_INSTALL_ICONS
macro(_KDE4_ADD_ICON_INSTALL_RULE _install_PATH _group _orig_NAME _install_NAME _l10n_SUBDIR)
    # if the string doesn't match the pattern, the result is the full string,
    # so all three have the same content
    if(NOT ${_group} STREQUAL ${_install_NAME})
        set(_icon_GROUP  ${_KDE4_ICON_GROUP_${_group}})
        if(NOT _icon_GROUP)
            set(_icon_GROUP "actions")
        endif()
        # message(STATUS "path: ${_install_PATH} group: ${_group} name: ${_install_NAME} l10n: ${_l10n_SUBDIR}")
        install(
            FILES ${_orig_NAME}
            DESTINATION ${_install_PATH}/${_icon_GROUP}/${_l10n_SUBDIR}/
            RENAME ${_install_NAME}
        )
    endif()
endmacro(_KDE4_ADD_ICON_INSTALL_RULE)

#  KDE4_INSTALL_ICONS(PATH THEME)
#    Installs all png and svgz files in the current directory to the icon
#    directory given in path, in the subdirectory for the given icon theme.
macro(KDE4_INSTALL_ICONS _defaultpath)
    # the l10n-subdir if language given as second argument (localized icon)
    set(_lang ${ARGV1})
    if(_lang)
        set(_l10n_SUBDIR l10n/${_lang})
    else()
        set(_l10n_SUBDIR ".")
    endif()

    # first the png icons
    file(GLOB _icons *.png)
    foreach(_current_ICON ${_icons} )
        # since CMake 2.6 regex matches are stored in special variables
        # CMAKE_MATCH_x, if it didn't match, they are empty
        string(
            REGEX MATCH "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.png)$"
            _dummy  "${_current_ICON}"
        )
        set(_type  "${CMAKE_MATCH_1}")
        set(_size  "${CMAKE_MATCH_2}")
        set(_group "${CMAKE_MATCH_3}")
        set(_name  "${CMAKE_MATCH_4}")

        set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
        if( _theme_GROUP)
            _KDE4_ADD_ICON_INSTALL_RULE(
                ${_defaultpath}/${_theme_GROUP}/${_size}x${_size}
                ${_group}
                ${_current_ICON}
                ${_name}
                ${_l10n_SUBDIR}
            )
        endif( _theme_GROUP)
    endforeach()

    # and now the svg icons
    file(GLOB _icons *.svgz)
    foreach(_current_ICON ${_icons})
        string(
            REGEX MATCH "^.*/([a-zA-Z]+)sc\\-([a-z]+)\\-(.+\\.svgz)$"
            _dummy "${_current_ICON}"
        )
        set(_type  "${CMAKE_MATCH_1}")
        set(_group "${CMAKE_MATCH_2}")
        set(_name  "${CMAKE_MATCH_3}")

        set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
        if(_theme_GROUP)
            _KDE4_ADD_ICON_INSTALL_RULE(
                ${_defaultpath}/${_theme_GROUP}/scalable
                ${_group}
                ${_current_ICON}
                ${_name}
                ${_l10n_SUBDIR}
            )
        endif()
    endforeach()
endmacro(KDE4_INSTALL_ICONS)

#  KDE4_ADD_KCFG_FILES(SRCS_VAR file1.kcfgc ... fileN.kcfgc)
#    Use this to add KDE config compiler files to your application/library.
macro(KDE4_ADD_KCFG_FILES _sources)
    foreach(_current_FILE ${ARGN})
        get_filename_component(_input ${_current_FILE} ABSOLUTE)
        get_filename_component(_abs_PATH ${_input} DIRECTORY)
        get_filename_component(_basename ${_input} NAME_WE)

        file(READ ${_input} _contents)
        string(REGEX REPLACE "^(.*\n)?File=([^\n]+kcfg).*\n.*$" "\\2"  _kcfg_FILENAME "${_contents}")
        set(_src_FILE    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
        set(_header_FILE ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
        set(_kcfg_FILE   ${_abs_PATH}/${_kcfg_FILENAME})
        # Maybe the .kcfg is a generated file?
        if(NOT EXISTS "${_kcfg_FILE}")
            set(_kcfg_FILE   ${CMAKE_CURRENT_BINARY_DIR}/${_kcfg_FILENAME})
        endif()
        if(NOT EXISTS "${_kcfg_FILE}")
            message(ERROR "${_kcfg_FILENAME} not found; tried in ${_abs_PATH} and ${CMAKE_CURRENT_BINARY_DIR}")
        endif()

        # make sure the directory exist in the build directory
        if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/")
            file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/")
        endif()

        # the command for creating the source file from the kcfg file
        add_custom_command(
            COMMAND ${KDE4_KCFGC_EXECUTABLE}
            ARGS ${_kcfg_FILE} ${_input} -d "${CMAKE_CURRENT_BINARY_DIR}/"
            MAIN_DEPENDENCY ${_input}
            DEPENDS ${_kcfg_FILE}
            OUTPUT ${_header_FILE} ${_src_FILE}
        )

        list(APPEND ${_sources} ${_src_FILE} ${_header_FILE})
    endforeach()
endmacro(KDE4_ADD_KCFG_FILES)

#  KDE4_ADD_PLUGIN(NAME FILE1 ... FILEN)
#    Create a KDE plugin (KPart, kioslave, etc.) from the given source files.
#    The resulting plugin will not have "lib" prefix.
macro(KDE4_ADD_PLUGIN _target)
    add_library(${_target} MODULE ${ARGN})

    set_target_properties(${_target} PROPERTIES PREFIX "")
endmacro(KDE4_ADD_PLUGIN)

#  KDE4_ADD_TEST(TESTNAME FILE1 ... FILEN)
#    add a unit test, which is executed when running make test. The targets
#    are build and executed only if the ENABLE_TESTING option is enabled.
#    KDESRCDIR is set to the source directory of the test, this can be used
#    with KGlobal::dirs()->addResourceDir( "data", KDESRCDIR )
macro(KDE4_ADD_TEST _target)
    KDE4_ADD_MANUAL_TEST(${_target} ${ARGN})

    add_test(
        NAME ${_target}
        COMMAND "${CMAKE_BINARY_DIR}/kde4_exec.sh" "${CMAKE_CURRENT_BINARY_DIR}/${_target}"
    )
endmacro(KDE4_ADD_TEST)

#  KDE4_ADD_MANUAL_TEST(TESTNAME FILE1 ... FILEN)
#    same as KDE_ADD_TEST() except that the test is not run on `make test`
macro(KDE4_ADD_MANUAL_TEST _target)
    add_executable(${_target} ${ARGN})

    target_compile_definitions(
        ${_target} PRIVATE
        -DKDESRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/"
        -DKDEBINDIR="${CMAKE_CURRENT_BINARY_DIR}/"
    )

    set_target_properties(
        ${_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endmacro(KDE4_ADD_MANUAL_TEST)

#  KDE4_ADD_WIDGET(SRCS_VAR FILE1.widgets ... FILEN.widgets)
#    Use this to add widget description files for the makekdewidgets code
#    generator for Qt Designer plugins.
macro(KDE4_ADD_WIDGET _sources)
    foreach(_current_FILE ${ARGN})
        get_filename_component(_input ${_current_FILE} ABSOLUTE)
        get_filename_component(_basename ${_input} NAME_WE)

        set(_output ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.cpp)
        set(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.moc)

        # create source file from the .widgets file
        if(_kdeBootStrapping)
            add_custom_command(
                COMMAND "${CMAKE_BINARY_DIR}/kde4_exec.sh" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KDE4_MAKEKDEWIDGETS_EXECUTABLE} -o ${_output} ${_input}
                MAIN_DEPENDENCY ${_input}
                DEPENDS ${KDE4_MAKEKDEWIDGETS_EXECUTABLE}
                OUTPUT ${_output}
            )
        else()
            add_custom_command(
                COMMAND ${KDE4_MAKEKDEWIDGETS_EXECUTABLE} -o ${_output} ${_input}
                MAIN_DEPENDENCY ${_input}
                OUTPUT ${_output}
            )
        endif()

        qt4_generate_moc("${_output}" "${_moc}")

        add_library(${_basename}_autowidgets OBJECT ${_output} ${_moc})

        set(${_sources} ${${_sources}} ${_output} ${_moc})
    endforeach()
endmacro(KDE4_ADD_WIDGET)

#  KDE4_INSTALL_AUTH_HELPER_FILES(HELPER_TARGET HELPER_ID)
#   This macro adds the needed files for an helper executable meant to be used
#   by applications using KAuth. It accepts the helper target and the helper ID
#   (the D-Bus name).
#
#   *WARNING* You have to install the helper in ${KDE4_LIBEXEC_INSTALL_DIR} to
#             make sure everything will work.
function(KDE4_INSTALL_AUTH_HELPER_FILES HELPER_TARGET HELPER_ID)
    if(_kdeBootStrapping)
        set(_stubFilesDir ${CMAKE_SOURCE_DIR}/kdecore)
    else()
        set(_stubFilesDir ${KDE4_DATA_INSTALL_DIR}/kauth)
    endif()

    configure_file(
        ${_stubFilesDir}/dbus_policy.stub
        ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.conf
    )
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.conf
        DESTINATION ${KDE4_SYSCONF_INSTALL_DIR}/dbus-1/system.d/
    )

    configure_file(
        ${_stubFilesDir}/dbus_service.stub
        ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.service
    )
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.service
        DESTINATION ${KDE4_DBUS_SYSTEM_SERVICES_INSTALL_DIR}
    )
endfunction(KDE4_INSTALL_AUTH_HELPER_FILES)

#  KDE4_ADD_DBUS_SERVICE(FILE1.service.in ... FILEN.service.in)
#    Use this to add D-Bus service activation file(s)
macro(KDE4_ADD_DBUS_SERVICE)
    foreach(_current_FILE ${ARGN})
        get_filename_component(_input ${_current_FILE} ABSOLUTE)
        get_filename_component(_basename ${_input} NAME)
        string(REPLACE ".service.in" ".service" _output ${_basename})

        configure_file(
            ${_input}
            ${CMAKE_CURRENT_BINARY_DIR}/${_output}
        )
        install(
            FILES ${CMAKE_CURRENT_BINARY_DIR}/${_output}
            DESTINATION ${KDE4_DBUS_SERVICES_INSTALL_DIR}
        )
    endforeach()
endmacro(KDE4_ADD_DBUS_SERVICE)

# KDE4_BOOL_TO_01(VAR RESULT)
#    This macro evaluates its first argument and sets the second argument
#    either to 0 or 1 depending on the value of the first one
macro(KDE4_BOOL_TO_01 _var _result)
    if(${_var})
        set(${_result} 1)
    else()
        set(${_result} 0)
    endif()
endmacro(KDE4_BOOL_TO_01)

# KDE4_OPTIONAL_ADD_SUBDIRECTORY(DIR)
#    If you use KDE4_OPTIONAL_ADD_SUBDIRECTORY() instead of ADD_SUBDIRECTORY()
#    an option to skip the subdirectory will be added. Skipping the directory
#    will be possbile via: cmake -DBUILD_<dir>=TRUE <srcdir>
macro(KDE4_OPTIONAL_ADD_SUBDIRECTORY _dir)
    option(BUILD_${_dir} "Build directory ${_dir}" TRUE)
    if(BUILD_${_dir})
        add_subdirectory(${_dir})
    endif()
endmacro(KDE4_OPTIONAL_ADD_SUBDIRECTORY)

# KDE4_OPTIONAL_FIND_PACKAGE(<PACKAGE> ...)
#    This macro is a combination of OPTION() and FIND_PACKAGE(), it works like
#    FIND_PACKAGE(), but additionally it automatically creates an option
#    WITH_<name>, which can be disabled via the cmake GUI or via
#    -DWITH_<name>=OFF
macro(KDE4_OPTIONAL_FIND_PACKAGE _PACKAGE)
   option(WITH_${_PACKAGE} "Search for ${_PACKAGE} package" ON)
   if (WITH_${_PACKAGE})
      find_package(${_PACKAGE} ${ARGN})
   endif ()
endmacro(KDE4_OPTIONAL_FIND_PACKAGE)

# KDE4_OPTIONAL_FIND_PACKAGE(<LANGUAGE> FILE1.po ... FILEN.po)
#    This macro is will create and install translation files
macro(KDE4_TRANSLATE _LANGUAGE)
    foreach(_pofile ${ARGN})
        get_filename_component(_abspofile "${_pofile}" ABSOLUTE)
        get_filename_component(_poname "${_abspofile}" NAME_WE)
        make_directory("${CMAKE_CURRENT_BINARY_DIR}")
        set(trout "${CMAKE_CURRENT_BINARY_DIR}/${_poname}.tr")
        string(REPLACE "@" "_" _language ${_LANGUAGE})
        add_custom_target(
            translations_${_language}_${_poname} ALL
            COMMAND ${KATIE_TRC} "${_abspofile}" -o "${trout}"
            COMMENT "Generating ${_poname}.tr"
            DEPENDS ${_abspofile}
        )
        set_source_files_properties("${trout}" PROPERTIES GENERATED TRUE)
        install(
            FILES "${trout}"
            DESTINATION "${KDE4_LOCALE_INSTALL_DIR}/${_LANGUAGE}"
            RENAME "${_poname}.tr"
        )
    endforeach()
endmacro(KDE4_TRANSLATE)
