# Try to find the shared-mime-info package, once done this will define:
#
#  SHAREDMIMEINFO_FOUND - system has the shared-mime-info package
#  UPDATE_MIME_DATABASE_EXECUTABLE - the update-mime-database executable
#
# The minimum required version of SharedMimeInfo can be specified using the
# standard syntax, e.g. find_package(SharedMimeInfo 0.91)
#
# For backward compatibility, the following two variables are also supported:
#  SHARED_MIME_INFO_MINIMUM_VERSION - set to the minimum version you need, default is 0.91.
#    When both are used, i.e. the version is set in the find_package() call and
#   SHARED_MIME_INFO_MINIMUM_VERSION is set, the version specified in the find_package()
#   call takes precedence.


# Copyright (c) 2007, Pino Toscano, <toscano.pino@tiscali.it>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# the minimum version of shared-mime-database we require
if(NOT SharedMimeInfo_FIND_VERSION)
    set(SharedMimeInfo_FIND_VERSION "0.91")
endif()

find_program (UPDATE_MIME_DATABASE_EXECUTABLE NAMES update-mime-database)

# Store the version number in the cache, so we don't have to search the next time again:
if (UPDATE_MIME_DATABASE_EXECUTABLE  AND NOT  SHAREDMIMEINFO_VERSION)
    execute_process(
        COMMAND ${UPDATE_MIME_DATABASE_EXECUTABLE} -v
        RETURN_VALUE _null
        ERROR_VARIABLE _smiVersionRaw
        OUTPUT_QUIET
    )

    string(REGEX REPLACE "update-mime-database \\([a-zA-Z\\-]+\\) ([0-9]\\.[0-9]+).*"
           "\\1" smiVersion "${_smiVersionRaw}")

    set(SHAREDMIMEINFO_VERSION "${smiVersion}" CACHE STRING "Version number of SharedMimeInfo" FORCE)
endif()

# Use the new FPHSA() syntax:
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SharedMimeInfo
    VERSION_VAR SHAREDMIMEINFO_VERSION
    REQUIRED_VARS UPDATE_MIME_DATABASE_EXECUTABLE
)

mark_as_advanced(UPDATE_MIME_DATABASE_EXECUTABLE)

macro(UPDATE_XDG_MIMETYPES _path)
    get_filename_component(_xdgmimeDir "${_path}" NAME)
    if("${_xdgmimeDir}" STREQUAL packages )
        get_filename_component(_xdgmimeDir "${_path}" PATH)
    else("${_xdgmimeDir}" STREQUAL packages )
        set(_xdgmimeDir "${_path}")
    endif("${_xdgmimeDir}" STREQUAL packages )

    install(CODE "
set(DESTDIR_VALUE \"\$ENV{DESTDIR}\")
if (NOT DESTDIR_VALUE)
  # under Windows relative paths are used, that's why it runs from CMAKE_INSTALL_PREFIX
  execute_process(COMMAND ${UPDATE_MIME_DATABASE_EXECUTABLE} ${_xdgmimeDir}
                  WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\")
endif()
")
endmacro()
