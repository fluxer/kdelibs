# Find if we installed kdebase/workspaces.
# Once done this will define
#
#  KDE4WORKSPACE_FOUND - system has KDE workspace installed
#  KDE4WORKSPACE_INCLUDE_DIR - the KDE workspace include directory
#
# It also sets variables for the following libraries:
#   KDE4WORKSPACE_TASKMANAGER_LIBRARY, KDE4WORKSPACE_TASKMANAGER_LIBS
#   KDE4WORKSPACE_KWORKSPACE_LIBRARY, KDE4WORKSPACE_KWORKSPACE_LIBS
#   KDE4WORKSPACE_PROCESSUI_LIBRARY, KDE4WORKSPACE_PROCESSUI_LIBS
#   KDE4WORKSPACE_LSOFUI_LIBRARY, KDE4WORKSPACE_LSOFUI_LIBS
#   KDE4WORKSPACE_PLASMACLOCK_LIBRARY, KDE4WORKSPACE_PLASMACLOCK_LIBS
#   KDE4WORKSPACE_KSCREENSAVER_LIBRARY, KDE4WORKSPACE_KSCREENSAVER_LIBS
#   KDE4WORKSPACE_WEATHERION_LIBRARY, KDE4WORKSPACE_WEATHERION_LIBS
#   KDE4WORKSPACE_KWINEFFECTS_LIBRARY, KDE4WORKSPACE_KWINEFFECTS_LIBS
#   KDE4WORKSPACE_KDECORATIONS_LIBRARY, KDE4WORKSPACE_KDECORATIONS_LIBS
#   KDE4WORKSPACE_KSGRD_LIBRARY, KDE4WORKSPACE_KSGRD_LIBS
#

# Copyright (c) 2008, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# The find_package() call below loads the file KDE4WorkspaceConfig.cmake file.
# This file is created and installed by kdebase/workspace/CMakeLists.txt
# It contains settings for all install location of kdebase/workspace, as e.g.
# KDE4WORKSPACE_INCLUDE_DIR, and also variables for all libraries.
# See kdebase/workspace/CMakeLists.txt and kdebase/workspace/KDE4WorkspaceConfig.cmake.in 
# for details. Alex


find_package(KDE4 REQUIRED)
set(_KDE4Workspace_FIND_QUIETLY  ${KDE4Workspace_FIND_QUIETLY})
find_package(KDE4Workspace QUIET NO_MODULE PATHS ${CMAKE_MODULE_PATH} )
set(KDE4Workspace_FIND_QUIETLY ${_KDE4Workspace_FIND_QUIETLY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(KDE4Workspace DEFAULT_MSG KDE4Workspace_CONFIG )

