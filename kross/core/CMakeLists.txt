include_directories(
    ${CMAKE_SOURCE_DIR}
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
)

set(krosscore_LIB_SRCS
    krossconfig.cpp
    interpreter.cpp
    script.cpp
    action.cpp
    actioncollection.cpp
    manager.cpp
    object.cpp
)

add_library(krosscore ${LIBRARY_TYPE} ${krosscore_LIB_SRCS})

target_link_libraries(krosscore PUBLIC
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTSCRIPT_LIBRARY}
    ${QT_QTXML_LIBRARY}
)

set_target_properties(krosscore PROPERTIES
    VERSION ${GENERIC_LIB_VERSION} 
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS krosscore
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

generate_export_header(krosscore BASE_NAME KROSS)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kross_export.h
    krossconfig.h
    errorinterface.h
    childreninterface.h
    wrapperinterface.h
    metatype.h
    metafunction.h
    interpreter.h
    script.h
    action.h
    actioncollection.h
    manager.h
    object.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kross/core
    COMPONENT Devel
)
