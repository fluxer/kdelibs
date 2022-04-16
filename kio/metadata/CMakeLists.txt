set(kfilemetadata_exiv2_SRCS kfilemetadata_exiv2.cpp)

kde4_add_plugin(kfilemetadata_exiv2 ${kfilemetadata_exiv2_SRCS})
target_link_libraries(kfilemetadata_exiv2
    ${KDE4_KIO_LIBS}
    ${KDE4_KEXIV2_LIBS}
)

install(
    TARGETS kfilemetadata_exiv2
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES kfilemetadata_exiv2.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)

if (FFMPEG_FOUND)
    include_directories(${FFMPEG_INCLUDES})

    set(kfilemetadata_ffmpeg_SRCS kfilemetadata_ffmpeg.cpp)

    kde4_add_plugin(kfilemetadata_ffmpeg ${kfilemetadata_ffmpeg_SRCS})
    target_link_libraries(kfilemetadata_ffmpeg
        ${KDE4_KIO_LIBS}
        ${FFMPEG_LIBRARIES}
    )

    install(
        TARGETS kfilemetadata_ffmpeg
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
    )

    install(
        FILES kfilemetadata_ffmpeg.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
    )
endif()

if (TAGLIB_FOUND)
    include_directories(${TAGLIB_INCLUDES})

    set(kfilemetadata_taglib_SRCS kfilemetadata_taglib.cpp)

    kde4_add_plugin(kfilemetadata_taglib ${kfilemetadata_taglib_SRCS})
    target_link_libraries(kfilemetadata_taglib
        ${KDE4_KIO_LIBS}
        ${TAGLIB_LIBRARIES}
    )

    install(
        TARGETS kfilemetadata_taglib
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
    )

    install(
        FILES kfilemetadata_taglib.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
    )
endif()

if (EPUB_FOUND)
    include_directories(${EPUB_INCLUDE_DIR})

    set(kfilemetadata_epub_SRCS kfilemetadata_epub.cpp)

    kde4_add_plugin(kfilemetadata_epub ${kfilemetadata_epub_SRCS})
    target_link_libraries(kfilemetadata_epub
        ${KDE4_KIO_LIBS}
        ${EPUB_LIBRARIES}
    )

    install(
        TARGETS kfilemetadata_epub
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
    )

    install(
        FILES kfilemetadata_epub.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
    )
endif()
