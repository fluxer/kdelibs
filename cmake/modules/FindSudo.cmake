# Try to find Sudo, once done this will define:
#
#  SUDO_FOUND - sudo has been found
#  SUDO_EXECUTABLE - the sudo executable path
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_program(SUDO_EXECUTABLE
    NAMES sudo
    HINTS ${KDE4_BIN_INSTALL_DIR}
    DOC "sudo -- execute a command as another user"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sudo
    REQUIRED_VARS SUDO_EXECUTABLE
)

mark_as_advanced(SUDO_EXECUTABLE)
