# for documentation look at FindKDE4Internal.cmake

# this file contains the following macros (or functions):
# KDE4_ADD_KCFG_FILES
# KDE4_ADD_PLUGIN
# KDE4_ADD_TEST
# KDE4_ADD_WIDGET
# KDE4_INSTALL_ICONS
# KDE4_CREATE_MANPAGE
# KDE4_INSTALL_AUTH_HELPER_FILES
# KDE4_AUTH_INSTALL_ACTIONS
# KDE4_INSTALL_TS_FILES

# Copyright (c) 2006-2009 Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
# Copyright (c) 2015 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


macro(KDE4_ADD_KCFG_FILES _sources )
    foreach(_current_ARG ${ARGN})
        if(${_current_ARG} STREQUAL "GENERATE_MOC" )
            set(_kcfg_generatemoc TRUE)
        endif()

        if(${_current_ARG} STREQUAL "USE_RELATIVE_PATH" )
            set(_kcfg_relativepath TRUE)
        endif()
    endforeach()

    foreach(_current_FILE ${ARGN})
        if(NOT ${_current_FILE} STREQUAL "GENERATE_MOC" AND NOT ${_current_FILE} STREQUAL "USE_RELATIVE_PATH")
            get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
            get_filename_component(_abs_PATH ${_tmp_FILE} DIRECTORY)

            if(_kcfg_relativepath) # Process relative path only if the option was set
                # Get relative path
                get_filename_component(_rel_PATH ${_current_FILE} DIRECTORY)

                if(IS_ABSOLUTE ${_rel_PATH})
                    # We got an absolute path
                    set(_rel_PATH "")
                endif()
            endif()

            get_filename_component(_basename ${_tmp_FILE} NAME_WE)
            # If we had a relative path and we're asked to use it, then change the basename accordingly
            if(NOT ${_rel_PATH} STREQUAL "")
                set(_basename ${_rel_PATH}/${_basename})
            endif()

            file(READ ${_tmp_FILE} _contents)
            string(REGEX REPLACE "^(.*\n)?File=([^\n]+kcfg).*\n.*$" "\\2"  _kcfg_FILENAME "${_contents}")
            set(_src_FILE    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
            set(_header_FILE ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
            set(_moc_FILE    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)
            set(_kcfg_FILE   ${_abs_PATH}/${_kcfg_FILENAME})
            # Maybe the .kcfg is a generated file?
            if(NOT EXISTS "${_kcfg_FILE}")
                set(_kcfg_FILE   ${CMAKE_CURRENT_BINARY_DIR}/${_kcfg_FILENAME})
            endif()
            if(NOT EXISTS "${_kcfg_FILE}")
                message(ERROR "${_kcfg_FILENAME} not found; tried in ${_abs_PATH} and ${CMAKE_CURRENT_BINARY_DIR}")
            endif()

            # make sure the directory exist in the build directory
            if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH}")
                file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH})
            endif()

            # if (CMAKE_CROSSCOMPILING)
            #     set(IMPORT_KCONFIG_COMPILER_EXECUTABLE "${KDE_HOST_TOOLS_PATH}/ImportKConfigCompilerExecutable.cmake"
            #         CACHE FILEPATH "Point it to the export file of kconfig_compiler from a native build")
            #     include(${IMPORT_KCONFIG_COMPILER_EXECUTABLE})
            #     set(KDE4_KCFGC_EXECUTABLE kconfig_compiler)
            # endif()

            # the command for creating the source file from the kcfg file
            add_custom_command(OUTPUT ${_header_FILE} ${_src_FILE}
                COMMAND ${KDE4_KCFGC_EXECUTABLE}
                ARGS ${_kcfg_FILE} ${_tmp_FILE} -d ${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH}
                MAIN_DEPENDENCY ${_tmp_FILE}
                DEPENDS ${_kcfg_FILE}
            )

            if(_kcfg_generatemoc)
                qt4_generate_moc(${_header_FILE} ${_moc_FILE} )
                set_source_files_properties(${_src_FILE} PROPERTIES SKIP_AUTOMOC TRUE)
                list(APPEND ${_sources} ${_moc_FILE})
            endif()

            list(APPEND ${_sources} ${_src_FILE} ${_header_FILE})
        endif(NOT ${_current_FILE} STREQUAL "GENERATE_MOC" AND NOT ${_current_FILE} STREQUAL "USE_RELATIVE_PATH")
    endforeach (_current_FILE)
endmacro(KDE4_ADD_KCFG_FILES)


get_filename_component(KDE4_MODULE_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)

macro(KDE4_CREATE_MANPAGE _docbook _section)
    get_filename_component(_input ${_docbook} ABSOLUTE)
    get_filename_component(_base ${_input} NAME)

    string(REGEX REPLACE "\\.${_section}\\.docbook$" "" _base ${_base})

    set(_doc ${CMAKE_CURRENT_BINARY_DIR}/${_base}.${_section})
    # sometimes we have "man-" prepended
    string(REGEX REPLACE "/man-" "/" _outdoc ${_doc})

    add_custom_command(OUTPUT ${_outdoc}
        COMMAND xsltproc ${_input}
        DEPENDS ${_input} xsltproc
    )
    get_filename_component(_targ ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set(_targ "${_targ}-manpage-${_base}")
    add_custom_target(${_targ} ALL DEPENDS "${_outdoc}")

    set(_args ${ARGN})

    set(_installDest)
    if(_args)
        list(GET _args 0 _tmp)
        if("${_tmp}" STREQUAL "INSTALL_DESTINATION")
            list(GET _args 1 _installDest )
            list(REMOVE_AT _args 0 1)
        endif("${_tmp}" STREQUAL "INSTALL_DESTINATION")
    endif(_args)

    get_filename_component(dirname ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)
    if(_args)
        list(GET _args 0 _tmp)
        if("${_tmp}" STREQUAL "SUBDIR")
            list(GET _args 1 dirname )
            list(REMOVE_AT _args 0 1)
        endif("${_tmp}" STREQUAL "SUBDIR")
    endif(_args)

    if(_installDest)
        install(FILES ${_outdoc} DESTINATION ${_installDest}/man${_section})
    endif(_installDest)
endmacro(KDE4_CREATE_MANPAGE)


# a "map" of short type names to the directories
# unknown names should give empty results
# KDE 3 compatibility
set(_KDE4_ICON_GROUP_mime       "mimetypes")
set(_KDE4_ICON_GROUP_filesys    "places")
set(_KDE4_ICON_GROUP_device     "devices")
set(_KDE4_ICON_GROUP_app        "apps")
set(_KDE4_ICON_GROUP_action     "actions")
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
set(_KDE4_ICON_THEME_lo "locolor")
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


macro(KDE4_INSTALL_ICONS _defaultpath )
    # the l10n-subdir if language given as second argument (localized icon)
    set(_lang ${ARGV1})
    if(_lang)
        set(_l10n_SUBDIR l10n/${_lang})
    else(_lang)
        set(_l10n_SUBDIR ".")
    endif(_lang)

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
    endforeach(_current_ICON)

    # mng icons
    file(GLOB _icons *.mng)
    foreach(_current_ICON ${_icons} )
        string(
            REGEX MATCH "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.mng)$"
            _dummy  "${_current_ICON}"
        )
        set(_type  "${CMAKE_MATCH_1}")
        set(_size  "${CMAKE_MATCH_2}")
        set(_group "${CMAKE_MATCH_3}")
        set(_name  "${CMAKE_MATCH_4}")

        set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
        if(_theme_GROUP)
            _KDE4_ADD_ICON_INSTALL_RULE(
                ${_defaultpath}/${_theme_GROUP}/${_size}x${_size}
                ${_group}
                ${_current_ICON}
                ${_name}
                ${_l10n_SUBDIR}
            )
        endif()
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


# If "WITH_PREFIX" is in the arugments then the standard "lib" prefix will be
# preserved
macro(KDE4_ADD_PLUGIN _target_NAME)
    set(_plugin_type MODULE)
    set(_plugin_prefix)
    set(_plugin_srcs)

    foreach(arg ${ARGN})
        if(arg STREQUAL "STATIC")
            set(_plugin_type STATIC)
        elseif(arg STREQUAL "SHARED")
            set(_plugin_type SHARED)
        elseif(arg STREQUAL "WITH_PREFIX")
            set(_plugin_prefix TRUE)
        else()
            list(APPEND _plugin_srcs ${arg})
        endif()
    endforeach()

    add_library(${_target_NAME} ${_plugin_type} ${_plugin_srcs})

    if("${_plugin_type}" STREQUAL STATIC)
        target_compile_definitions(${_target_NAME} PRIVATE -DQT_STATICPLUGIN)
    endif()

    if(NOT "${_plugin_prefix}")
        set_target_properties(${_target_NAME} PROPERTIES PREFIX "")
    endif()
endmacro(KDE4_ADD_PLUGIN)

# Add a unit test, which is executed when running make test. The targets are
# always created and built unless ENABLE_TESTING is set to negative value.
macro(KDE4_ADD_TEST _targetName)
    KDE4_ADD_MANUAL_TEST(${_targetName} ${ARGN})
    add_test(NAME ${_targetName} COMMAND ${_targetName})
endmacro(KDE4_ADD_TEST)

macro(KDE4_ADD_MANUAL_TEST _targetName)
    add_executable(${_targetName} ${ARGN})

    target_compile_definitions(
        ${_targetName} PRIVATE
        -DKDESRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/"
        -DKDEBINDIR="${CMAKE_CURRENT_BINARY_DIR}/"
    )
    set_target_properties(
        ${_targetName} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endmacro(KDE4_ADD_MANUAL_TEST)

macro(KDE4_ADD_WIDGET _output _sources)
    foreach(_current_FILE ${_sources})
        get_filename_component(_input ${_current_FILE} ABSOLUTE)
        get_filename_component(_basename ${_input} NAME_WE)

        set(_source ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.cpp)
        set(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.moc)

        # create source file from the .widgets file
        add_custom_command(
            OUTPUT ${_source}
            COMMAND ${KDE4_MAKEKDEWIDGETS_EXECUTABLE} -o ${_source} ${_input}
            MAIN_DEPENDENCY ${_input}
        )

        qt4_generate_moc("${_source}" "${_moc}")

        add_library(${_basename}_autowidgets OBJECT ${_source} ${_moc})

        set(${_output} ${${_output}} ${_source} ${_moc})
    endforeach(_current_FILE)
endmacro(KDE4_ADD_WIDGET)


# This macro adds the needed files for an helper executable meant to be used by
# applications using KAuth. It accepts the helper target, the helper ID (the
# DBUS name) and the user under which the helper will run on. The macro also
# takes care of generate the needed files, and install them in the right
# location. This boils down to a DBus policy to let the helper register on the
# system bus, and a service file for letting the helper being automatically
# activated by the system bus.
#
# *WARNING* You have to install the helper in ${LIBEXEC_INSTALL_DIR} to make
# sure everything will work.
function(KDE4_INSTALL_AUTH_HELPER_FILES HELPER_TARGET HELPER_ID HELPER_USER)
    if (_kdeBootStrapping)
        set(_stubFilesDir ${CMAKE_SOURCE_DIR}/kdecore/auth/backends/dbus/)
    else (_kdeBootStrapping)
        set(_stubFilesDir ${KDE4_DATA_INSTALL_DIR}/kauth/)
    endif (_kdeBootStrapping)

    configure_file(${_stubFilesDir}/dbus_policy.stub
                    ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.conf)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.conf
            DESTINATION ${SYSCONF_INSTALL_DIR}/dbus-1/system.d/)

    configure_file(${_stubFilesDir}/dbus_service.stub
                    ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.service)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.service
            DESTINATION ${DBUS_SYSTEM_SERVICES_INSTALL_DIR})
endfunction(KDE4_INSTALL_AUTH_HELPER_FILES)


macro(KDE4_INSTALL_TS_FILES _lang _sdir)
    file(GLOB_RECURSE _ts_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${_sdir}/*)
    foreach(_current_TS_FILES ${_ts_files})
        if(NOT ${_current_TS_FILES} MATCHES ".git/")
            get_filename_component(_subpath ${_current_TS_FILES} PATH)
            install(
                FILES ${_current_TS_FILES}
                DESTINATION ${LOCALE_INSTALL_DIR}/${_lang}/LC_SCRIPTS/${_subpath}
            )
        endif()
    endforeach()
endmacro(KDE4_INSTALL_TS_FILES)
