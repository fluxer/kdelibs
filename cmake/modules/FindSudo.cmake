# - Try to find Sudo
# Once done this will define
#
#  SUDO_FOUND - sudo has been found
#  SUDO_EXECUTABLE - the sudo executable path
#

find_program(SUDO_EXECUTABLE NAMES sudo
    HINTS ${BIN_INSTALL_DIR}
    DOC "sudo -- execute a command as another user"
)

if(SUDO_EXECUTABLE)
    set(SUDO_FOUND TRUE)
    message(STATUS "Found Sudo: ${SUDO_EXECUTABLE}")
else(SUDO_EXECUTABLE)
   set(SUDO_FOUND FALSE)
   message(STATUS "Sudo not found.")
endif(SUDO_EXECUTABLE)
