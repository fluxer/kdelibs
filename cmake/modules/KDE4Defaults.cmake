# let cmake handle mocking and UI compiling
# since cmake 2.8.6
set(CMAKE_AUTOMOC ON)
# since cmake 3.0.0
set(CMAKE_AUTOUIC ON)

# always include srcdir and builddir in include path, this saves typing
# ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} in about every subdir
# since cmake 2.4.0
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# put the include dirs which are in the source or build tree before all other
# include dirs, so the headers in the sources are preferred over the already
# installed ones
# since cmake 2.4.1
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# do not duplicate interface includes as system from projects providing
# imported targets in case the include directories are already included
# explicitly (via include_directories() which always uses non-system style),
# reduces the command-line arguments that have to be passed to the compiler and
# most likely the build times
set(CMAKE_NO_SYSTEM_FROM_IMPORTED ON)

# skip re-linking during installation
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# define the generic version of the libraries here, this makes it easy to
# advance it when the next KDE release comes. Use this version number for
# libraries
set(GENERIC_LIB_VERSION "4.20")
set(GENERIC_LIB_SOVERSION "4")
