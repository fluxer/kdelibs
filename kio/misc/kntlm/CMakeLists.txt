set(kntlm_LIB_SRCS kntlm.cpp des.cpp)

add_library(kntlm ${LIBRARY_TYPE} ${kntlm_LIB_SRCS})

target_link_libraries(kntlm ${KDE4_KDECORE_LIBS})

set_target_properties(kntlm PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS kntlm
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

generate_export_header(kntlm)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kntlm_export.h
    kntlm.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kio
    COMPONENT Devel
)

