# let cmake handle mocking and UI compiling
# since cmake 2.8.6
set(CMAKE_AUTOMOC ON)
# since cmake 3.0.0
set(CMAKE_AUTOUIC ON)

# Always include srcdir and builddir in include path, this saves typing
# ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} in about every subdir
# since cmake 2.4.0
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# put the include dirs which are in the source or build tree before all other
# include dirs, so the headers in the sources are preferred over the already
# installed ones
# since cmake 2.4.1
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# define the generic version of the libraries here, this makes it easy to
# advance it when the next KDE release comes. Use this version number for
# libraries
set(GENERIC_LIB_VERSION "4.19.0")
set(GENERIC_LIB_SOVERSION "4")

# Use this version number for libraries which are experimental
set(KDE_NON_GENERIC_LIB_VERSION "5.15.0")
set(KDE_NON_GENERIC_LIB_SOVERSION "5")

