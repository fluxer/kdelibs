# this is required by cmake >=2.6
cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

# CMP0000: don't require cmake_minimum_version() directly in the top level
# CMakeLists.txt, KDELibs4Config.cmake is good enough
cmake_policy(SET CMP0000 OLD)
# CMP0003: add the link paths to the link command as with cmake 2.4
cmake_policy(SET CMP0003 OLD)
if(NOT CMAKE_VERSION VERSION_LESS "3.3.0")
    # CMP0003: enable symbols visibility preset for all targets
    cmake_policy(SET CMP0063 NEW)
endif()
if(NOT CMAKE_VERSION VERSION_LESS "3.10.0")
    cmake_policy(SET CMP0071 OLD)
endif()

# let cmake handle mocking and UI compiling
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# always include srcdir and builddir in include path, this saves typing
# ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} in about every subdir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# put the include dirs which are in the source or build tree before all other
# include dirs, so the headers in the sources are preferred over the already
# installed ones
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

if(ENABLE_TESTING)
    enable_testing()
endif()
