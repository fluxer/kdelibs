# Try to find OpenGLES, once done this will define:
#  
#  OPENGLES_FOUND           - system has OpenGLES and EGL
#  OPENGL_EGL_FOUND         - system has EGL
#  OPENGLES_INCLUDE_DIR     - the GLES include directory
#  OPENGLES_LIBRARY	    - the GLES library
#  OPENGLES_EGL_INCLUDE_DIR - the EGL include directory
#  OPENGLES_EGL_LIBRARY	    - the EGL library
#  OPENGLES_LIBRARIES       - all libraries needed for OpenGLES
#  OPENGLES_INCLUDES        - all includes needed for OpenGLES
#
# Copyright (c) 2021 Ivailo Monev <xakepa10@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(OPENGLES_INCLUDE_DIR
    NAMES GLES2/gl2.h
    HINTS /usr/X11R7/include /usr/X11R6/include
)

find_library(OPENGLES_LIBRARY
    NAMES GLESv2
    HINTS /usr/X11R7/lib /usr/X11R6/lib
)

find_path(OPENGLES_EGL_INCLUDE_DIR
    NAMES EGL/egl.h
    HINTS /usr/X11R7/include /usr/X11R6/include
)

find_library(OPENGLES_EGL_LIBRARY
    NAMES EGL
    HINTS /usr/X11R7/lib /usr/X11R6/lib
)

set(OPENGL_EGL_FOUND "NO")
if(OPENGLES_EGL_LIBRARY AND OPENGLES_EGL_INCLUDE_DIR)
    set(OPENGL_EGL_FOUND "YES")
endif()

set(OPENGLES_FOUND "NO")
if(OPENGLES_LIBRARY AND OPENGLES_INCLUDE_DIR AND OPENGLES_EGL_LIBRARY AND OPENGLES_EGL_INCLUDE_DIR)
    set(OPENGLES_LIBRARIES ${OPENGLES_LIBRARY} ${OPENGLES_EGL_LIBRARY})
    set(OPENGLES_INCLUDES ${OPENGLES_INCLUDE_DIR} ${OPENGLES_EGL_INCLUDE_DIR})
    set(OPENGLES_FOUND "YES")
endif()

