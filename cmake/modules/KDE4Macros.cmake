# for documentation look at FindKDE4Internal.cmake

# this file contains the following macros (or functions):
# KDE4_ADD_UI_FILES
# KDE4_ADD_KCFG_FILES
# _KDE4_SET_CUSTOM_TARGET_PROPERTY
# _KDE4_GET_CUSTOM_TARGET_PROPERTY
# KDE4_ADD_PLUGIN
# KDE4_ADD_KDEINIT_EXECUTABLE
# KDE4_ADD_UNIT_TEST
# KDE4_ADD_EXECUTABLE
# KDE4_ADD_WIDGET_FILES
# KDE4_UPDATE_ICONCACHE
# KDE4_INSTALL_ICONS
# KDE4_REMOVE_OBSOLETE_CMAKE_FILES
# KDE4_ADD_APP_ICON
# KDE4_CREATE_MANPAGE
# KDE4_CREATE_BASIC_CMAKE_VERSION_FILE (function)
# KDE4_INSTALL_AUTH_HELPER_FILES
# KDE4_AUTH_INSTALL_ACTIONS

# Copyright (c) 2006-2009 Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


macro (KDE4_ADD_KCFG_FILES _sources )
   foreach (_current_ARG ${ARGN})
       if( ${_current_ARG} STREQUAL "USE_RELATIVE_PATH" )
           set(_kcfg_relativepath TRUE)
       endif( ${_current_ARG} STREQUAL "USE_RELATIVE_PATH" )
   endforeach (_current_ARG ${ARGN})

   foreach (_current_FILE ${ARGN})

     if(NOT ${_current_FILE} STREQUAL "GENERATE_MOC" AND NOT ${_current_FILE} STREQUAL "USE_RELATIVE_PATH")
       get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
       get_filename_component(_abs_PATH ${_tmp_FILE} PATH)

       if (_kcfg_relativepath) # Process relative path only if the option was set
           # Get relative path
           get_filename_component(_rel_PATH ${_current_FILE} PATH)

           if (IS_ABSOLUTE ${_rel_PATH})
               # We got an absolute path
               set(_rel_PATH "")
           endif (IS_ABSOLUTE ${_rel_PATH})
       endif (_kcfg_relativepath)

       get_filename_component(_basename ${_tmp_FILE} NAME_WE)
       # If we had a relative path and we're asked to use it, then change the basename accordingly
       if(NOT ${_rel_PATH} STREQUAL "")
           set(_basename ${_rel_PATH}/${_basename})
       endif(NOT ${_rel_PATH} STREQUAL "")

       file(READ ${_tmp_FILE} _contents)
       string(REGEX REPLACE "^(.*\n)?File=([^\n]+kcfg).*\n.*$" "\\2"  _kcfg_FILENAME "${_contents}")
       set(_src_FILE    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
       set(_header_FILE ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
       set(_moc_FILE    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)
       set(_kcfg_FILE   ${_abs_PATH}/${_kcfg_FILENAME})
       # Maybe the .kcfg is a generated file?
       if(NOT EXISTS "${_kcfg_FILE}")
           set(_kcfg_FILE   ${CMAKE_CURRENT_BINARY_DIR}/${_kcfg_FILENAME})
       endif(NOT EXISTS "${_kcfg_FILE}")
       if(NOT EXISTS "${_kcfg_FILE}")
           message(ERROR "${_kcfg_FILENAME} not found; tried in ${_abs_PATH} and ${CMAKE_CURRENT_BINARY_DIR}")
       endif(NOT EXISTS "${_kcfg_FILE}")

       # make sure the directory exist in the build directory
       if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH}")
           file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH})
       endif(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH}")

#       if (CMAKE_CROSSCOMPILING)
#           set(IMPORT_KCONFIG_COMPILER_EXECUTABLE "${KDE_HOST_TOOLS_PATH}/ImportKConfigCompilerExecutable.cmake" CACHE FILEPATH "Point it to the export file of kconfig_compiler from a native build")
#           include(${IMPORT_KCONFIG_COMPILER_EXECUTABLE})
#           set(KDE4_KCFGC_EXECUTABLE kconfig_compiler)
#       endif (CMAKE_CROSSCOMPILING)

       # the command for creating the source file from the kcfg file
       add_custom_command(OUTPUT ${_header_FILE} ${_src_FILE}
          COMMAND ${KDE4_KCFGC_EXECUTABLE}
          ARGS ${_kcfg_FILE} ${_tmp_FILE} -d ${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH}
          MAIN_DEPENDENCY ${_tmp_FILE}
          DEPENDS ${_kcfg_FILE} ${_KDE4_KCONFIG_COMPILER_DEP} )

       list(APPEND ${_sources} ${_src_FILE} ${_header_FILE})
     endif(NOT ${_current_FILE} STREQUAL "GENERATE_MOC" AND NOT ${_current_FILE} STREQUAL "USE_RELATIVE_PATH")
   endforeach (_current_FILE)

endmacro (KDE4_ADD_KCFG_FILES)


get_filename_component(KDE4_MODULE_DIR  ${CMAKE_CURRENT_LIST_FILE} PATH)

#create the implementation files from the ui files and add them to the list of sources
#usage: KDE4_ADD_UI_FILES(foo_SRCS ${ui_files})
macro (KDE4_ADD_UI_FILES _sources )
   foreach (_current_FILE ${ARGN})

      get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_tmp_FILE} NAME_WE)
      set(_header ${CMAKE_CURRENT_BINARY_DIR}/ui_${_basename}.h)

      # we need to run uic and replace some things in the generated file
      # this is done by executing the cmake script kde4uic.cmake
      add_custom_command(OUTPUT ${_header}
         COMMAND ${CMAKE_COMMAND}
         ARGS
         -DKDE4_HEADER:BOOL=ON
         -DKDE_UIC_EXECUTABLE:FILEPATH=${QT_UIC_EXECUTABLE}
         -DKDE_UIC_FILE:FILEPATH=${_tmp_FILE}
         -DKDE_UIC_H_FILE:FILEPATH=${_header}
         -DKDE_UIC_BASENAME:STRING=${_basename}
         -P ${KDE4_MODULE_DIR}/kde4uic.cmake
         MAIN_DEPENDENCY ${_tmp_FILE}
      )
      list(APPEND ${_sources} ${_header})
   endforeach (_current_FILE)
endmacro (KDE4_ADD_UI_FILES)


# this is basically a copy of the qt4_get_moc_flags() macros from FindQt4.cmake
# which is for internal use only, so we should not use it here:
macro (_KDE4_GET_MOC_FLAGS _moc_flags)
   set(${_moc_flags})
   get_directory_property(_inc_DIRS INCLUDE_DIRECTORIES)

   foreach(_current ${_inc_DIRS})
      set(${_moc_flags} ${${_moc_flags}} "-I${_current}")
   endforeach(_current ${_inc_DIRS})

   get_directory_property(_defines COMPILE_DEFINITIONS)
   foreach(_current ${_defines})
      set(${_moc_flags} ${${_moc_flags}} "-D${_current}")
   endforeach(_current ${_defines})

   # if Qt is installed only as framework, add -F /library/Frameworks to the moc arguments
   # otherwise moc can't find the headers in the framework include dirs
   if(APPLE  AND  "${QT_QTCORE_INCLUDE_DIR}" MATCHES "/Library/Frameworks/")
      set(${_moc_INC_DIRS} ${${_moc_INC_DIRS}} "-F/Library/Frameworks")
   endif(APPLE  AND  "${QT_QTCORE_INCLUDE_DIR}" MATCHES "/Library/Frameworks/")

endmacro(_KDE4_GET_MOC_FLAGS)


macro (_KDE4_SET_CUSTOM_TARGET_PROPERTY _target_name _property_name _property)
   string(REGEX REPLACE "[/ ]" "_" _dir "${CMAKE_CURRENT_SOURCE_DIR}")
   set(_kde4_${_dir}_${_target_name}_${_property_name} "${_property}")
endmacro (_KDE4_SET_CUSTOM_TARGET_PROPERTY)


macro (_KDE4_GET_CUSTOM_TARGET_PROPERTY _var _target_name _property_name)
   string(REGEX REPLACE "[/ ]" "_" _dir "${CMAKE_CURRENT_SOURCE_DIR}")
   set(${_var} "${_kde4_${_dir}_${_target_name}_${_property_name}}")
endmacro (_KDE4_GET_CUSTOM_TARGET_PROPERTY)

macro(KDE4_INSTALL_TS_FILES _lang _sdir)
   file(GLOB_RECURSE _ts_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${_sdir}/*)
   foreach(_current_TS_FILES ${_ts_files})
      string(REGEX MATCH "\\.svn/" _in_svn ${_current_TS_FILES})
      if(NOT _in_svn)
         get_filename_component(_subpath ${_current_TS_FILES} PATH)
         install(FILES ${_current_TS_FILES} DESTINATION ${LOCALE_INSTALL_DIR}/${_lang}/LC_SCRIPTS/${_subpath})
      endif(NOT _in_svn)
   endforeach(_current_TS_FILES)
endmacro(KDE4_INSTALL_TS_FILES)

macro (KDE4_CREATE_MANPAGE _docbook _section)
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
endmacro (KDE4_CREATE_MANPAGE)


macro (KDE4_UPDATE_ICONCACHE)
    # Update mtime of hicolor icon theme dir.
    # We don't always have touch command (e.g. on Windows), so instead create
    #  and delete a temporary file in the theme dir.
   install(CODE "
    set(DESTDIR_VALUE \"\$ENV{DESTDIR}\")
    if (NOT DESTDIR_VALUE)
        file(WRITE \"${ICON_INSTALL_DIR}/hicolor/temp.txt\" \"update\")
        file(REMOVE \"${ICON_INSTALL_DIR}/hicolor/temp.txt\")
    endif (NOT DESTDIR_VALUE)
    ")
endmacro (KDE4_UPDATE_ICONCACHE)

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
set(_KDE4_ICON_THEME_cr "crystalsvg")
set(_KDE4_ICON_THEME_lo "locolor")
set(_KDE4_ICON_THEME_hi "hicolor")


# only used internally by KDE4_INSTALL_ICONS
macro (_KDE4_ADD_ICON_INSTALL_RULE _install_SCRIPT _install_PATH _group _orig_NAME _install_NAME _l10n_SUBDIR)

   # if the string doesn't match the pattern, the result is the full string, so all three have the same content
   if (NOT ${_group} STREQUAL ${_install_NAME} )
      set(_icon_GROUP  ${_KDE4_ICON_GROUP_${_group}})
      if(NOT _icon_GROUP)
         set(_icon_GROUP "actions")
      endif(NOT _icon_GROUP)
#      message(STATUS "icon: ${_current_ICON} size: ${_size} group: ${_group} name: ${_name} l10n: ${_l10n_SUBDIR}")
      install(FILES ${_orig_NAME} DESTINATION ${_install_PATH}/${_icon_GROUP}/${_l10n_SUBDIR}/ RENAME ${_install_NAME} )
   endif (NOT ${_group} STREQUAL ${_install_NAME} )

endmacro (_KDE4_ADD_ICON_INSTALL_RULE)


macro (KDE4_INSTALL_ICONS _defaultpath )

   # the l10n-subdir if language given as second argument (localized icon)
   set(_lang ${ARGV1})
   if(_lang)
      set(_l10n_SUBDIR l10n/${_lang})
   else(_lang)
      set(_l10n_SUBDIR ".")
   endif(_lang)

   # first the png icons
   file(GLOB _icons *.png)
   foreach (_current_ICON ${_icons} )
      # since CMake 2.6 regex matches are stored in special variables CMAKE_MATCH_x, if it didn't match, they are empty
      string(REGEX MATCH "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" _dummy  "${_current_ICON}")
      set(_type  "${CMAKE_MATCH_1}")
      set(_size  "${CMAKE_MATCH_2}")
      set(_group "${CMAKE_MATCH_3}")
      set(_name  "${CMAKE_MATCH_4}")

      set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
      if( _theme_GROUP)
         _KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                    ${_defaultpath}/${_theme_GROUP}/${_size}x${_size}
                    ${_group} ${_current_ICON} ${_name} ${_l10n_SUBDIR})
      endif( _theme_GROUP)
   endforeach (_current_ICON)

   # mng icons
   file(GLOB _icons *.mng)
   foreach (_current_ICON ${_icons} )
      # since CMake 2.6 regex matches are stored in special variables CMAKE_MATCH_x, if it didn't match, they are empty
      string(REGEX MATCH "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.mng)$" _dummy  "${_current_ICON}")
      set(_type  "${CMAKE_MATCH_1}")
      set(_size  "${CMAKE_MATCH_2}")
      set(_group "${CMAKE_MATCH_3}")
      set(_name  "${CMAKE_MATCH_4}")

      set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
      if( _theme_GROUP)
         _KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                ${_defaultpath}/${_theme_GROUP}/${_size}x${_size}
                ${_group} ${_current_ICON} ${_name} ${_l10n_SUBDIR})
      endif( _theme_GROUP)
   endforeach (_current_ICON)

   # and now the svg icons
   file(GLOB _icons *.svgz)
   foreach (_current_ICON ${_icons} )
      # since CMake 2.6 regex matches are stored in special variables CMAKE_MATCH_x, if it didn't match, they are empty
      string(REGEX MATCH "^.*/([a-zA-Z]+)sc\\-([a-z]+)\\-(.+\\.svgz)$" _dummy "${_current_ICON}")
      set(_type  "${CMAKE_MATCH_1}")
      set(_group "${CMAKE_MATCH_2}")
      set(_name  "${CMAKE_MATCH_3}")

      set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
      if( _theme_GROUP)
          _KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                            ${_defaultpath}/${_theme_GROUP}/scalable
                            ${_group} ${_current_ICON} ${_name} ${_l10n_SUBDIR})
      endif( _theme_GROUP)
   endforeach (_current_ICON)

   kde4_update_iconcache()

endmacro (KDE4_INSTALL_ICONS)


# This macro doesn't set up the RPATH related options for executables anymore,
# since now (wioth cmake 2.6) just the full RPATH is used always for everything.
# It does create wrapper shell scripts for the executables.
# It overrides the defaults set in FindKDE4Internal.cmake.
# For every executable a wrapper script is created, which sets the appropriate
# environment variable for the platform (LD_LIBRARY_PATH on most UNIX systems,
# DYLD_LIBRARY_PATH on OS X and PATH in Windows) so  that it points to the built
# but not yet installed versions of the libraries. So if RPATH is disabled, the executables
# can be run via these scripts from the build tree and will find the correct libraries.
# If RPATH is not disabled, these scripts are also used but only for consistency, because
# they don't really influence anything then, because the compiled-in RPATH overrides
# the LD_LIBRARY_PATH env. variable.
macro (KDE4_HANDLE_RPATH_FOR_EXECUTABLE _target_NAME)
   if (UNIX)
      if (APPLE)
         set(_library_path_variable "DYLD_LIBRARY_PATH")
      elseif (CYGWIN)
         set(_library_path_variable "PATH")
      else (APPLE)
         set(_library_path_variable "LD_LIBRARY_PATH")
      endif (APPLE)

      if (APPLE)
         # DYLD_LIBRARY_PATH does not work like LD_LIBRARY_PATH
         # OSX already has the RPATH in libraries and executables, putting runtime directories in
         # DYLD_LIBRARY_PATH actually breaks things
         set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/:${KDE4_LIB_DIR}")
      else (APPLE)
         set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/:${LIB_INSTALL_DIR}:${KDE4_LIB_DIR}:${QT_LIBRARY_DIR}")
      endif (APPLE)
      get_target_property(_executable ${_target_NAME} LOCATION )

      # use add_custom_target() to have the sh-wrapper generated during build time instead of cmake time
      if (CMAKE_VERSION VERSION_GREATER 2.8.4)
         add_custom_command(TARGET ${_target_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -D_filename=${_executable}.shell -D_library_path_variable=${_library_path_variable}
            -D_ld_library_path="${_ld_library_path}" -D_executable=$<TARGET_FILE:${_target_NAME}>
            -P ${KDE4_MODULE_DIR}/kde4_exec_via_sh.cmake
            )
      else ()
         add_custom_command(TARGET ${_target_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -D_filename=${_executable}.shell -D_library_path_variable=${_library_path_variable}
            -D_ld_library_path="${_ld_library_path}" -D_executable=${_executable}
            -P ${KDE4_MODULE_DIR}/kde4_exec_via_sh.cmake
            )
      endif ()

      macro_additional_clean_files(${_executable}.shell)

      # under UNIX, set the property WRAPPER_SCRIPT to the name of the generated shell script
      # so it can be queried and used later on easily
      set_target_properties(${_target_NAME} PROPERTIES WRAPPER_SCRIPT ${_executable}.shell)

   else (UNIX)
      # under windows, set the property WRAPPER_SCRIPT just to the name of the executable
      # maybe later this will change to a generated batch file (for setting the PATH so that the Qt libs are found)
      get_target_property(_executable ${_target_NAME} LOCATION )
      set_target_properties(${_target_NAME} PROPERTIES WRAPPER_SCRIPT ${_executable})

      set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}\;${LIB_INSTALL_DIR}\;${KDE4_LIB_DIR}\;${QT_LIBRARY_DIR}")
      get_target_property(_executable ${_target_NAME} LOCATION )

      # use add_custom_target() to have the batch-file-wrapper generated during build time instead of cmake time
      add_custom_command(TARGET ${_target_NAME} POST_BUILD
         COMMAND ${CMAKE_COMMAND}
         -D_filename="${_executable}.bat"
         -D_ld_library_path="${_ld_library_path}" -D_executable="${_executable}"
         -P ${KDE4_MODULE_DIR}/kde4_exec_via_sh.cmake
         )

   endif (UNIX)
endmacro (KDE4_HANDLE_RPATH_FOR_EXECUTABLE)


macro (KDE4_ADD_PLUGIN _target_NAME )
#if the first argument is "WITH_PREFIX" then keep the standard "lib" prefix,
#otherwise set the prefix empty

   set(_args ${ARGN})
   # default to module
   set(_add_lib_param "MODULE")
   set(_with_pre FALSE)
   
   foreach(arg ${_args})
      if (arg STREQUAL "WITH_PREFIX")
         set(_with_pre TRUE)
      endif (arg STREQUAL "WITH_PREFIX")
      if (arg STREQUAL "STATIC")
         set(_add_lib_param STATIC)
      endif (arg STREQUAL "STATIC")
      if (arg STREQUAL "SHARED")
         set(_add_lib_param SHARED)
      endif (arg STREQUAL "SHARED")
      if (arg STREQUAL "MODULE")
         set(_add_lib_param MODULE)
      endif (arg STREQUAL "MODULE")
   endforeach(arg)

   if(_with_pre)
      list(REMOVE_ITEM _args "WITH_PREFIX")
   endif(_with_pre)
   if(_add_lib_param STREQUAL "STATIC")
      list(REMOVE_ITEM _args "STATIC")
   endif(_add_lib_param STREQUAL "STATIC")
   if (_add_lib_param STREQUAL "SHARED")
      list(REMOVE_ITEM _args "SHARED")
   endif (_add_lib_param STREQUAL "SHARED")
   if (_add_lib_param STREQUAL "MODULE")
       list(REMOVE_ITEM _args "MODULE")
   endif (_add_lib_param STREQUAL "MODULE")

   set(_SRCS ${_args})

   if("${_add_lib_param}" STREQUAL "STATIC")
      add_definitions(-DQT_STATICPLUGIN)
   endif("${_add_lib_param}" STREQUAL "STATIC")

   add_library(${_target_NAME} ${_add_lib_param}  ${_SRCS})

   if (NOT _with_pre)
      set_target_properties(${_target_NAME} PROPERTIES PREFIX "")
   endif (NOT _with_pre)

   # for shared libraries/plugins a -DMAKE_target_LIB is required
   string(TOUPPER ${_target_NAME} _symbol)
   string(REGEX REPLACE "[^_A-Za-z0-9]" "_" _symbol ${_symbol})
   set(_symbol "MAKE_${_symbol}_LIB")
   set_target_properties(${_target_NAME} PROPERTIES DEFINE_SYMBOL ${_symbol})

endmacro (KDE4_ADD_PLUGIN _target_NAME _with_PREFIX)


# this macro is intended to check whether a list of source
# files has the "NOGUI" or "RUN_UNINSTALLED" keywords at the beginning
# in _output_LIST the list of source files is returned with the "NOGUI"
# and "RUN_UNINSTALLED" keywords removed
# if "NOGUI" is in the list of files, the _nogui argument is set to
# "NOGUI" (which evaluates to TRUE in cmake), otherwise it is set empty
# (which evaluates to FALSE in cmake)
# "RUN_UNINSTALLED" in the list of files is ignored, it is not necessary anymore
# since KDE 4.2 (with cmake 2.6.2), since then all executables are always built
# with RPATH pointing into the build dir.
# if "TEST" is in the list of files, the _test argument is set to
# "TEST" (which evaluates to TRUE in cmake), otherwise it is set empty
# (which evaluates to FALSE in cmake)
macro(KDE4_CHECK_EXECUTABLE_PARAMS _output_LIST _nogui _test)
   set(${_nogui})
   set(${_test})
   set(${_output_LIST} ${ARGN})
   list(LENGTH ${_output_LIST} count)

   list(GET ${_output_LIST} 0 first_PARAM)

   set(second_PARAM "NOTFOUND")
   if (${count} GREATER 1)
      list(GET ${_output_LIST} 1 second_PARAM)
   endif (${count} GREATER 1)

   set(remove "NOTFOUND")

   if (${first_PARAM} STREQUAL "NOGUI")
      set(${_nogui} "NOGUI")
      set(remove 0)
   endif (${first_PARAM} STREQUAL "NOGUI")

   if (${first_PARAM} STREQUAL "RUN_UNINSTALLED")
      set(remove 0)
   endif (${first_PARAM} STREQUAL "RUN_UNINSTALLED")

   if (${first_PARAM} STREQUAL "TEST")
      set(${_test} "TEST")
      set(remove 0)
   endif (${first_PARAM} STREQUAL "TEST")

   if (${second_PARAM} STREQUAL "NOGUI")
      set(${_nogui} "NOGUI")
      set(remove 0;1)
   endif (${second_PARAM} STREQUAL "NOGUI")

   if (${second_PARAM} STREQUAL "RUN_UNINSTALLED")
      set(remove 0;1)
   endif (${second_PARAM} STREQUAL "RUN_UNINSTALLED")

   if (${second_PARAM} STREQUAL "TEST")
      set(${_test} "TEST")
      set(remove 0;1)
   endif (${second_PARAM} STREQUAL "TEST")


   if (NOT "${remove}" STREQUAL "NOTFOUND")
      list(REMOVE_AT ${_output_LIST} ${remove})
   endif (NOT "${remove}" STREQUAL "NOTFOUND")

endmacro(KDE4_CHECK_EXECUTABLE_PARAMS)


macro (KDE4_ADD_KDEINIT_EXECUTABLE _target_NAME )

   kde4_check_executable_params(_SRCS _nogui _test ${ARGN})

   configure_file(${KDE4_MODULE_DIR}/kde4init_dummy.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_dummy.cpp)
   set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_dummy.cpp PROPERTIES SKIP_AUTOMOC TRUE)
   # under Windows, build a normal executable and additionally a dummy kdeinit4_foo.lib, whose only purpose on windows is to 
   # keep the linking logic from the CMakeLists.txt on UNIX working (under UNIX all necessary libs are linked against the kdeinit
   # library instead against the executable, under windows we want to have everything in the executable, but for compatibility we have to 
   # keep the library there-

   add_library(kdeinit_${_target_NAME} SHARED ${_SRCS})

   set_target_properties(kdeinit_${_target_NAME} PROPERTIES OUTPUT_NAME kdeinit4_${_target_NAME})

   kde4_add_executable(${_target_NAME} "${_nogui}" ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_dummy.cpp ${_resourcefile})
   target_link_libraries(${_target_NAME} kdeinit_${_target_NAME})
endmacro (KDE4_ADD_KDEINIT_EXECUTABLE)

# Add a unit test, which is executed when running make test .
# The targets are always created, but only built for the "all"
# target if the option KDE4_BUILD_TESTS is enabled. Otherwise the rules for the target
# are created but not built by default. You can build them by manually building the target.
# The name of the target can be specified using TESTNAME <testname>, if it is not given
# the macro will default to the <name>
macro (KDE4_ADD_UNIT_TEST _test_NAME)
    set(_srcList ${ARGN})
    set(_targetName ${_test_NAME})
    if( ${ARGV1} STREQUAL "TESTNAME" )
        set(_targetName ${ARGV2})
        list(REMOVE_AT _srcList 0 1)
    endif( ${ARGV1} STREQUAL "TESTNAME" )

    set(_nogui)
    list(GET _srcList 0 first_PARAM)
    if( ${first_PARAM} STREQUAL "NOGUI" )
        set(_nogui "NOGUI")
    endif( ${first_PARAM} STREQUAL "NOGUI" )

    kde4_add_executable( ${_test_NAME} TEST ${_srcList} )

    if(NOT KDE4_TEST_OUTPUT)
        set(KDE4_TEST_OUTPUT plaintext)
    endif(NOT KDE4_TEST_OUTPUT)
    set(KDE4_TEST_OUTPUT ${KDE4_TEST_OUTPUT} CACHE STRING "The output to generate when running the QTest unit tests")

    set(using_qtest "")
    foreach(_filename ${_srcList})
        if(NOT using_qtest)
            if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_filename}")
                file(READ ${_filename} file_CONTENT)
                string(REGEX MATCH "QTEST_(KDE)?MAIN" using_qtest "${file_CONTENT}")
            endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_filename}")
        endif(NOT using_qtest)
    endforeach(_filename)

    get_target_property( loc ${_test_NAME} LOCATION )

    if (using_qtest AND KDE4_TEST_OUTPUT STREQUAL "xml")
        #MESSAGE(STATUS "${_targetName} : Using QTestLib, can produce XML report.")
        add_test( ${_targetName} ${_executable} -xml -o ${_targetName}.tml)
    else (using_qtest AND KDE4_TEST_OUTPUT STREQUAL "xml")
        #MESSAGE(STATUS "${_targetName} : NOT using QTestLib, can't produce XML report, please use QTestLib to write your unit tests.")
        add_test( ${_targetName} ${_executable} )
    endif (using_qtest AND KDE4_TEST_OUTPUT STREQUAL "xml")

#    add_test( ${_targetName} ${EXECUTABLE_OUTPUT_PATH}/${_test_NAME} -xml -o ${_test_NAME}.tml )

    # if the tests are EXCLUDE_FROM_ALL, add a target "buildtests" to build all tests
    if (NOT KDE4_BUILD_TESTS)
        get_directory_property(_buildtestsAdded BUILDTESTS_ADDED)
        if(NOT _buildtestsAdded)
            add_custom_target(buildtests)
            set_directory_properties(PROPERTIES BUILDTESTS_ADDED TRUE)
        endif(NOT _buildtestsAdded)
        add_dependencies(buildtests ${_test_NAME})
    endif (NOT KDE4_BUILD_TESTS)

endmacro (KDE4_ADD_UNIT_TEST)


# add a  manifest file to executables. 
# 
# There is a henn-egg problem when a target runtime part is renamed using 
# the OUTPUT_NAME option of cmake's set_target_properties command. 
#
# At now the Makefiles rules creating for manifest adding are performed 
# *after* the cmake's add_executable command but *before* an optional 
# set_target_properties command. 
# This means that in KDE4_ADD_MANIFEST the LOCATION property contains 
#  the unchanged runtime part name of the target. :-(
# 
# The recently used workaround is to specify a variable build off the target name followed 
# by _OUTPUT_NAME before calling kde4_add_executable as shown in the following example: 
# 
# set(xyz_OUTPUT_NAME test)
# kde4_add_executable( xyz <source>)
# set_target_properties( xyz PROPERTIES OUTPUT_NAME ${xyz_OUTPUT_NAME} )  
#
# The full solution would be to introduce a kde4_target_link_libraries macro and to 
# call KDE4_ADD_MANIFEST inside instead of calling in kde4_add_executable. 
# This would require patching of *all* places in the KDE sources where target_link_libraries 
# is used and to change the related docs.
# 
# Because yet I found only 2 locations where this problem occurs (kjs, k3b), the workaround 
# seems to be a pragmatically solution. 
# 
# This macro is an internal macro only used by kde4_add_executable
#
macro (_KDE4_ADD_MANIFEST _target_NAME)
    set(x ${_target_NAME}_OUTPUT_NAME)
    if (${x})
        get_target_property(_var ${_target_NAME} LOCATION )
        string(REPLACE "${_target_NAME}" "${${x}}" _executable ${_var})
    else(${x})
        get_target_property(_executable ${_target_NAME} LOCATION )
    endif(${x})

    if (_kdeBootStrapping)
        set(_cmake_module_path ${CMAKE_SOURCE_DIR}/cmake/modules)
    else (_kdeBootStrapping)
        set(_cmake_module_path ${KDE4_INSTALL_DIR}/share/apps/cmake/modules)
    endif (_kdeBootStrapping)

    set(_manifest ${_cmake_module_path}/Win32.Manifest.in)
    #message(STATUS ${_executable} ${_manifest})
    add_custom_command(
        TARGET ${_target_NAME}
        POST_BUILD
        COMMAND ${KDE4_MT_EXECUTABLE}
        ARGS
           -manifest ${_manifest}
           -updateresource:${_executable}
        COMMENT "adding vista trustInfo manifest to ${_target_NAME}"
   )
endmacro(_KDE4_ADD_MANIFEST) 


macro (KDE4_ADD_EXECUTABLE _target_NAME)

   kde4_check_executable_params( _SRCS _nogui _test ${ARGN})

   set(_add_executable_param)

   if (_nogui)
      set(_add_executable_param)
   endif (_nogui)

   if (_test AND NOT KDE4_BUILD_TESTS)
      set(_add_executable_param ${_add_executable_param} EXCLUDE_FROM_ALL)
   endif (_test AND NOT KDE4_BUILD_TESTS)

   add_executable(${_target_NAME} ${_add_executable_param} ${_SRCS})

   IF (KDE4_ENABLE_UAC_MANIFEST)
       _kde4_add_manifest(${_target_NAME})
   ENDIF(KDE4_ENABLE_UAC_MANIFEST)

   if (_test)
      set_target_properties(${_target_NAME} PROPERTIES COMPILE_FLAGS -DKDESRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"")
   endif (_test)

   kde4_handle_rpath_for_executable(${_target_NAME})
endmacro (KDE4_ADD_EXECUTABLE)


macro (KDE4_ADD_LIBRARY _target_NAME _lib_TYPE)

   set(_first_SRC ${_lib_TYPE})
   set(_add_lib_param)

   if (${_lib_TYPE} STREQUAL "STATIC")
      set(_first_SRC)
      set(_add_lib_param STATIC)
   endif (${_lib_TYPE} STREQUAL "STATIC")
   if (${_lib_TYPE} STREQUAL "SHARED")
      set(_first_SRC)
      set(_add_lib_param SHARED)
   endif (${_lib_TYPE} STREQUAL "SHARED")
   if (${_lib_TYPE} STREQUAL "MODULE")
      set(_first_SRC)
      set(_add_lib_param MODULE)
   endif (${_lib_TYPE} STREQUAL "MODULE")

   set(_SRCS ${_first_SRC} ${ARGN})

   add_library(${_target_NAME} ${_add_lib_param} ${_SRCS})

   # for shared libraries a -DMAKE_target_LIB is required
   string(TOUPPER ${_target_NAME} _symbol)
   string(REGEX REPLACE "[^_A-Za-z0-9]" "_" _symbol ${_symbol})
   set(_symbol "MAKE_${_symbol}_LIB")
   set_target_properties(${_target_NAME} PROPERTIES DEFINE_SYMBOL ${_symbol})

   # By default don't add any linked libraries to the "exported"
   # link interfaces, so that executables linking against this library
   # will not automatically add implicit dependencies to their link list.
   #
   # This reduces inter-package dependencies and makes it easier to remove
   # dependencies of shared libraries without breaking binary compatibility.
   if(NOT "${_add_lib_param}" STREQUAL "STATIC")
      set_target_properties(${_target_NAME} PROPERTIES LINK_INTERFACE_LIBRARIES "" )
   endif(NOT "${_add_lib_param}" STREQUAL "STATIC")

endmacro (KDE4_ADD_LIBRARY _target_NAME _lib_TYPE)

macro (KDE4_ADD_WIDGET_FILES _sources)
   foreach (_current_FILE ${ARGN})

      get_filename_component(_input ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_input} NAME_WE)
      set(_source ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.cpp)
      set(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.moc)

      # create source file from the .widgets file
      add_custom_command(OUTPUT ${_source}
        COMMAND ${KDE4_MAKEKDEWIDGETS_EXECUTABLE}
        ARGS -o ${_source} ${_input}
        MAIN_DEPENDENCY ${_input} DEPENDS ${_KDE4_MAKEKDEWIDGETS_DEP} ${KDE4_MAKEKDEWIDGETS_EXECUTABLE})

      # create moc file
      qt4_generate_moc(${_source} ${_moc} )

      list(APPEND ${_sources} ${_source} ${_moc})

   endforeach (_current_FILE)

endmacro (KDE4_ADD_WIDGET_FILES)


macro(KDE4_REMOVE_OBSOLETE_CMAKE_FILES)
# the files listed here will be removed by remove_obsoleted_cmake_files.cmake, Alex
   install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/remove_files.cmake )
   set(module_install_dir ${DATA_INSTALL_DIR}/cmake/modules )

   file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/remove_files.cmake "#generated by cmake, don't edit\n\n")
   foreach ( _current_FILE ${ARGN})
      file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/remove_files.cmake "message(STATUS \"Removing ${module_install_dir}/${_current_FILE}\" )\n" )
      file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/remove_files.cmake "exec_program( ${CMAKE_COMMAND} ARGS -E remove ${module_install_dir}/${_current_FILE} OUTPUT_VARIABLE _dummy)\n" )
   endforeach ( _current_FILE)

endmacro(KDE4_REMOVE_OBSOLETE_CMAKE_FILES)


macro(KDE4_CREATE_EXPORTS_HEADER _outputFile _libName)
   string(TOUPPER ${_libName} _libNameUpperCase)
   string(REGEX REPLACE "[^_A-Za-z0-9]" "_" _libNameUpperCase ${_libNameUpperCase})
   # the next line is is required, because in CMake arguments to macros are not real
   # variables, but handled differently. The next line create a real CMake variable,
   # so configure_file() will replace it correctly.
   set(_libName ${_libName})
   # compared to write(FILE) configure_file() only really writes the file if the
   # contents have changed. Otherwise we would have a lot of recompiles.
   configure_file(${KDE4_MODULE_DIR}/kde4exportsheader.h.in ${_outputFile})
endmacro(KDE4_CREATE_EXPORTS_HEADER _outputFile _libName)

# adds application icon to target source list 
# for detailed documentation see the top of FindKDE4Internal.cmake
macro (KDE4_ADD_APP_ICON appsources pattern)
    set (_outfilename ${CMAKE_CURRENT_BINARY_DIR}/${appsources})
endmacro (KDE4_ADD_APP_ICON)


# This macro is only kept around for compatibility, it is not needed/used anymore
# since CMake 2.6.0. With CMake 2.6.0 it is not necessary anymore link libraries again
# ("relink") to change their RPATH. Since this is fast now, they are now always built with
# full RPATH. 
# Still keep this macro here, since somebody might use it and so that would break
# if we would just remove it from here.
# What it does now it sets the target properties of the given target the same way as
# they were set by the old version of the macro with the option FULL_RPATH enabled.
# This one may be a candidate for removal. Alex
macro (KDE4_HANDLE_RPATH_FOR_LIBRARY _target_NAME)
   message(STATUS "You are using the macro KDE4_HANDLE_RPATH_FOR_LIBRARY(), which is an internal macro and shouldn't be used by external projects. Please remove it.")
   if (NOT CMAKE_SKIP_RPATH)
      set_target_properties(${_target_NAME} PROPERTIES  SKIP_BUILD_RPATH FALSE  BUILD_WITH_INSTALL_RPATH FALSE)
   endif (NOT CMAKE_SKIP_RPATH)
endmacro (KDE4_HANDLE_RPATH_FOR_LIBRARY)

# This macro adds the needed files for an helper executable meant to be used by applications using KAuth.
# It accepts the helper target, the helper ID (the DBUS name) and the user under which the helper will run on.
# This macro takes care of generate the needed files, and install them in the right location. This boils down
# to a DBus policy to let the helper register on the system bus, and a service file for letting the helper
# being automatically activated by the system bus.
# *WARNING* You have to install the helper in ${LIBEXEC_INSTALL_DIR} to make sure everything will work.
function(KDE4_INSTALL_AUTH_HELPER_FILES HELPER_TARGET HELPER_ID HELPER_USER)
    if (_kdeBootStrapping)
        set(_stubFilesDir  ${CMAKE_SOURCE_DIR}/kdecore/auth/backends/dbus/ )
    else (_kdeBootStrapping)
        set(_stubFilesDir  ${KDE4_DATA_INSTALL_DIR}/kauth/ )
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

# This macro generates an action file, depending on the backend used, for applications using KAuth.
# It accepts the helper id (the DBUS name) and a file containing the actions (check kdelibs/kdecore/auth/example
# for file format). The macro will take care of generating the file according to the backend specified, 
# and to install it in the right location. This (at the moment) means that on Linux (PolicyKit) a .policy
# file will be generated and installed into the policykit action directory (usually /usr/share/PolicyKit/policy/),
# and on Mac (Authorization Services) will be added to the system action registry using the native MacOS API during
# the install phase
function(KDE4_INSTALL_AUTH_ACTIONS HELPER_ID ACTIONS_FILE)
  message(AUTHOR_WARNING "PolicyKit/Polikt actions are not required")
endfunction(KDE4_INSTALL_AUTH_ACTIONS)


macro(_KDE4_EXPORT_LIBRARY_DEPENDENCIES _append_or_write _filename)
   message(FATAL_ERROR "_KDE4_EXPORT_LIBRARY_DEPENDENCIES() was an internal macro and has been removed again. Just remove the code which calls it, there is no substitute.")
endmacro(_KDE4_EXPORT_LIBRARY_DEPENDENCIES)

macro (_KDE4_TARGET_LINK_INTERFACE_LIBRARIES _target _interface_libs)
   message(FATAL_ERROR "_KDE4_TARGET_LINK_INTERFACE_LIBRARIES() doesn't exist anymore. Set the LINK_INTERFACE_LIBRARIES target property instead. See kdelibs/kdecore/CMakeLists.txt for an example.")
endmacro (_KDE4_TARGET_LINK_INTERFACE_LIBRARIES)

macro (KDE4_TARGET_LINK_INTERFACE_LIBRARIES _target _interface_libs)
   message(FATAL_ERROR "KDE4_TARGET_LINK_INTERFACE_LIBRARIES() doesn't exist anymore. Set the LINK_INTERFACE_LIBRARIES target property instead. See kdelibs/kdecore/CMakeLists.txt for an example.")
endmacro (KDE4_TARGET_LINK_INTERFACE_LIBRARIES _target _interface_libs)

macro (KDE4_SET_CUSTOM_TARGET_PROPERTY)
   message(FATAL_ERROR "KDE4_SET_CUSTOM_TARGET_PROPERTY() is deprecated, just use a simple variable instead")
endmacro (KDE4_SET_CUSTOM_TARGET_PROPERTY)

macro (KDE4_GET_CUSTOM_TARGET_PROPERTY)
   message(FATAL_ERROR "KDE4_GET_CUSTOM_TARGET_PROPERTY() is deprecated, just use a simple variable instead")
endmacro (KDE4_GET_CUSTOM_TARGET_PROPERTY)
