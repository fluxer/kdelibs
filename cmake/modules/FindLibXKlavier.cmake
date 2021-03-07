# Try to find LibXKlavier, once done this will define:
#
#  LIBXKLAVIER_FOUND - system has LibXKlavier
#  LIBXKLAVIER_LDFLAGS - the libraries needed to use LibXKlavier
#  LIBXKLAVIER_CFLAGS - Compiler switches required for using LibXKlavier
#  LIBXKLAVIER_VERSION - Version of LibXKlavier
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT WIN32)
    include(FindPkgConfig)
    pkg_check_modules(LIBXKLAVIER QUIET libxklavier>=3.0)
endif()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibXKlavier
    VERSION_VAR LIBXKLAVIER_VERSION
    REQUIRED_VARS LIBXKLAVIER_CFLAGS LIBXKLAVIER_LDFLAGS
)

mark_as_advanced(LIBXKLAVIER_CFLAGS LIBXKLAVIER_LDFLAGS)
