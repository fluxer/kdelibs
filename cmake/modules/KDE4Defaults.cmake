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

# do not export symbols that are not public
set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")

# place all binaries in the same directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
# place all libraries, static and shared, in the same directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# big number, ensures all source files are included from one unity file
set(CMAKE_UNITY_BUILD_BATCH_SIZE 200)

# define the generic version of the libraries here, this makes it easy to
# advance it when the next KDE release comes. Use this version number for
# libraries
set(GENERIC_LIB_VERSION "4.23")
set(GENERIC_LIB_SOVERSION "4")
