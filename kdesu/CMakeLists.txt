project(kdesu)

include_directories(${KDE4_KPTY_INCLUDES})

check_function_exists(setpriority HAVE_SETPRIORITY)
check_function_exists(getpeereid HAVE_GETPEEREID)

set(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
# defines HAVE_STRUCT_UCRED (bool) and STRUCT_UCRED (size, unused)
check_type_size("struct ucred" STRUCT_UCRED)
# reset CMAKE_EXTRA_INCLUDE_FILES
set(CMAKE_EXTRA_INCLUDE_FILES)

configure_file(
    config-kdesu.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kdesu.h
)

find_package(Sudo)
set_package_properties(Sudo PROPERTIES
    DESCRIPTION "Sudo allows a system administrator to delegate authority to give certain users"
    URL "http://www.sudo.ws/"
    PURPOSE "Needed for password caching in kdesu"
)

########### next target ###############

set(kdesu_LIB_SRCS
    client.cpp
    process.cpp
    kcookie.cpp
    su.cpp
    ssh.cpp
    stub.cpp
)

add_library(kdesu ${LIBRARY_TYPE} ${kdesu_LIB_SRCS})

target_link_libraries(kdesu PRIVATE ${KDE4_KPTY_LIBS})
target_link_libraries(kdesu PUBLIC ${QT_QTCORE_LIBRARY})

if(SUDO_FOUND)
    add_definitions(-DKDESU_USE_SUDO_DEFAULT)
endif()

set_target_properties(kdesu PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS kdesu
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

########### next target ###############

set(kdesu_stub_SRCS kdesu_stub.c )

add_executable(kdesu_stub ${kdesu_stub_SRCS})

install(TARGETS kdesu_stub DESTINATION ${LIBEXEC_INSTALL_DIR})

########### install files ###############

install(
    FILES
    kdesu_export.h
    client.h
    process.h
    su.h
    ssh.h
    stub.h
    defaults.h 
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdesu
    COMPONENT Devel
)
