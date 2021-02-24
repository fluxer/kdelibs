# - Try to find the PAM libraries
# Once done this will define
#
#  PAM_FOUND - system has pam
#  PAM_INCLUDE_DIR - the pam include directory
#  PAM_LIBRARIES - libpam library
#  PAM_MESSAGE_CONST - libpam pam_message struct is const
#
# Copyright (c) 2021, Ivailo Monev, <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(PAM_INCLUDE_DIR NAMES security/pam_appl.h)
find_library(PAM_LIBRARY pam)
find_library(DL_LIBRARY dl)

if(DL_LIBRARY)
    set(PAM_LIBRARIES ${PAM_LIBRARY} ${DL_LIBRARY})
else()
    set(PAM_LIBRARIES ${PAM_LIBRARY})
endif()

if(NOT DEFINED PAM_MESSAGE_CONST)
    include(CheckCXXSourceCompiles)
    include(CMakePushCheckState)
    cmake_reset_check_state()
    set(CMAKE_REQUIRED_INCLUDES "${PAM_INCLUDE_DIR}")
    # XXX does this work with plain c?
    check_cxx_source_compiles("
#include <security/pam_appl.h>

static int PAM_conv(
    int num_msg,
    const struct pam_message **msg, /* this is the culprit */
    struct pam_response **resp,
    void *ctx)
{
    return 0;
}

int main(void)
{
    struct pam_conv PAM_conversation = {
        &PAM_conv, /* this bombs out if the above does not match */
        0
    };

    return 0;
}
" PAM_MESSAGE_CONST)
        cmake_reset_check_state()
endif()
set(PAM_MESSAGE_CONST ${PAM_MESSAGE_CONST} CACHE BOOL "PAM expects a conversation function with const pam_message")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PAM
    REQUIRED_VARS PAM_LIBRARIES PAM_INCLUDE_DIR
)

mark_as_advanced(PAM_INCLUDE_DIR PAM_LIBRARY DL_LIBRARY PAM_MESSAGE_CONST)
