project(threadweaver)

if(ENABLE_TESTING)
    add_subdirectory(Tests)
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=950)
include_directories(${KDE4_KDECORE_INCLUDES})

add_subdirectory(Weaver)
# disabled for default kdelibs build:
# add_subdirectory(Examples)

generate_export_header(threadweaver)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/threadweaver_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver
    COMPONENT Devel
)
