include_directories(
    ${CMAKE_SOURCE_DIR}/kross
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
)

add_executable(kross main.cpp)

target_link_libraries(kross ${KDE4_KDEUI_LIBS} krosscore)

install(TARGETS kross ${INSTALL_TARGETS_DEFAULT_ARGS})
