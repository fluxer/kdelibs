set(kfilemetadata_exiv2_SRCS kfilemetadata_exiv2.cpp)

kde4_add_plugin(kfilemetadata_exiv2 ${kfilemetadata_exiv2_SRCS})
target_link_libraries(kfilemetadata_exiv2
    kio
    kexiv2
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
        ${FFMPEG_LIBRARIES}
        kio
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
        ${TAGLIB_LIBRARIES}
        kio
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
        ${EPUB_LIBRARIES}
        kio
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

if (POPPLER_FOUND)
    include_directories(${POPPLER_INCLUDE_DIR})

    set(kfilemetadata_poppler_SRCS kfilemetadata_poppler.cpp)

    kde4_add_plugin(kfilemetadata_poppler ${kfilemetadata_poppler_SRCS})
    target_link_libraries(kfilemetadata_poppler
        ${POPPLER_LIBRARIES}
        kio
        karchive
    )

    install(
        TARGETS kfilemetadata_poppler
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
    )

    install(
        FILES kfilemetadata_poppler.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
    )
endif()

if (LIBSPECTRE_FOUND)
    include_directories(${LIBSPECTRE_INCLUDE_DIR})

    set(kfilemetadata_spectre_SRCS kfilemetadata_spectre.cpp)

    kde4_add_plugin(kfilemetadata_spectre ${kfilemetadata_spectre_SRCS})
    target_link_libraries(kfilemetadata_spectre
        ${LIBSPECTRE_LIBRARY}
        kio
    )

    install(
        TARGETS kfilemetadata_spectre
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
    )

    install(
        FILES kfilemetadata_spectre.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
    )
endif()

if (FREETYPE_FOUND)
    include_directories(${FREETYPE_INCLUDE_DIRS})

    set(kfilemetadata_freetype_SRCS kfilemetadata_freetype.cpp)

    kde4_add_plugin(kfilemetadata_freetype ${kfilemetadata_freetype_SRCS})
    target_link_libraries(kfilemetadata_freetype
        ${FREETYPE_LIBRARIES}
        kio
    )

    install(
        TARGETS kfilemetadata_freetype
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
    )

    install(
        FILES kfilemetadata_freetype.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
    )
endif()

if (DJVULIBRE_FOUND)
    include_directories(${DJVULIBRE_INCLUDE_DIR})

    set(kfilemetadata_djvulibre_SRCS kfilemetadata_djvulibre.cpp)

    kde4_add_plugin(kfilemetadata_djvulibre ${kfilemetadata_djvulibre_SRCS})
    target_link_libraries(kfilemetadata_djvulibre
        ${DJVULIBRE_LIBRARY}
        kio
    )

    install(
        TARGETS kfilemetadata_djvulibre
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
    )

    install(
        FILES kfilemetadata_djvulibre.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
    )
endif()
