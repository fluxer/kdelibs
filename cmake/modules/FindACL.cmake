# Try to find the ACL library, once done this will define:
#
#  ACL_FOUND - system has the ACL library
#  ACL_LIBS - The libraries needed to use ACL

# Copyright (c) 2006, Pino Toscano, <toscano.pino@tiscali.it>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(CheckIncludeFiles)

check_include_files(sys/acl.h HAVE_SYS_ACL_H)
check_include_files(acl/libacl.h HAVE_ACL_LIBACL_H)

if(HAVE_SYS_ACL_H AND HAVE_ACL_LIBACL_H)
    set(ACL_HEADERS_FOUND TRUE)
endif()

if (ACL_HEADERS_FOUND)
   find_library(ACL_LIBS NAMES acl)
endif()

if(ACL_HEADERS_FOUND AND ACL_LIBS)
    set(ACL_FOUND TRUE)
    set(ACL_LIBS ${ACL_LIBS})
    message(STATUS "Found ACL support: ${ACL_LIBS}")
endif()

mark_as_advanced(ACL_LIBS)

