project(kioslave-file)

include_directories(${KDE4_KIO_INCLUDES})

include(ConfigureChecks.cmake)
configure_file(
    config-kioslave-file.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kioslave-file.h
)

########### next target ###############

set(kio_file_PART_SRCS file.cpp file_unix.cpp)

kde4_add_plugin(kio_file ${kio_file_PART_SRCS})

target_link_libraries(kio_file ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS})

if(ACL_FOUND)
    target_link_libraries(kio_file ${ACL_LIBS})
endif()

install(TARGETS kio_file DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES file.protocol DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

generate_export_header(kio_file)
