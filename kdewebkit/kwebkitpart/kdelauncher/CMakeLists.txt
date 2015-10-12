project(kdelauncher)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

include_directories(
    ${KDE4_INCLUDES}
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

add_executable(kdelauncher main.cpp)
target_link_libraries(kdelauncher
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEWEBKIT_LIBS}
)
