include_directories(
    ${CMAKE_SOURCE_DIR}
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
)

set(krossui_LIB_SRCS
    model.cpp
    view.cpp
    plugin.cpp
    # guiclient.cpp
)

add_library(krossui ${LIBRARY_TYPE} ${krossui_LIB_SRCS})
target_link_libraries(krossui PUBLIC
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KIO_LIBS}
    krosscore
)
set_target_properties(krossui PROPERTIES
    VERSION ${GENERIC_LIB_VERSION} 
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

########### install files ###############

install(
    TARGETS krossui
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(
    FILES
    # guiclient.h
    model.h
    view.h
    plugin.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/kross/ui
    COMPONENT Devel
)
