project(kioslave-file)

include_directories(${KDE4_KIO_INCLUDES})

include(ConfigureChecks.cmake)
configure_file(
    config-kioslave-file.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kioslave-file.h
)

########### next target ###############

set(kio_file_PART_SRCS file.cpp file_unix.cpp)

add_executable(kio_file ${kio_file_PART_SRCS})

target_link_libraries(kio_file kdecore kio)

if(ACL_FOUND)
    target_link_libraries(kio_file ${ACL_LIBS})
endif()

install(TARGETS kio_file DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR})


########### install files ###############

install(FILES file.protocol DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
