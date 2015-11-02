project(kunitconversion)

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(KUNITCONVERSION_NO_SOLID TRUE)
    set(KUNITCONVERSION_NO_KIO TRUE)
endif()

configure_file(
    config-kunitconversion.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kunitconversion.h
)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

set(kunitconversion_LIB_SRCS
    converter.cpp
    value.cpp
    unit.cpp
    unitcategory.cpp
    length.cpp
    area.cpp
    volume.cpp
    temperature.cpp
    velocity.cpp
    mass.cpp
    pressure.cpp
    energy.cpp
    currency.cpp
    power.cpp
    timeunit.cpp
    fuel_efficiency.cpp
    density.cpp
    acceleration.cpp
    force.cpp
    angle.cpp
    frequency.cpp
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=930)

if(NOT KUNITCONVERSION_NO_KIO)
    include_directories(${KDE4_KIO_INCLUDES})
    set(KUNITCONVERSION_EXTRA_LIBS
        ${KUNITCONVERSION_EXTRA_LIBS}
        ${KDE4_KIO_LIBS}
    )
else()
    set(KUNITCONVERSION_EXTRA_LIBS
        ${KUNITCONVERSION_EXTRA_LIBS}
        ${QT_QTNETWORK_LIBRARY}
        ${QT_QTXML_LIBRARY}
    )
endif()

if(NOT KUNITCONVERSION_NO_SOLID)
    include_directories(
        ${CMAKE_BINARY_DIR}/solid
        ${CMAKE_SOURCE_DIR}/solid
    )
    set(KUNITCONVERSION_EXTRA_LIBS
        ${KUNITCONVERSION_EXTRA_LIBS}
        ${KDE4_SOLID_LIBS}
    )
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
)

add_library(kunitconversion ${LIBRARY_TYPE} ${kunitconversion_LIB_SRCS})

target_link_libraries(kunitconversion
    ${KDE4_KDECORE_LIBS}
    ${KUNITCONVERSION_EXTRA_LIBS}
)

set_target_properties(kunitconversion PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS kunitconversion
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(
    FILES
    converter.h
    value.h
    unit.h
    unitcategory.h
    kunitconversion_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kunitconversion
    COMPONENT Devel
)
