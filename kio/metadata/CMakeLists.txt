set(kfilemetadata_exiv2_SRCS kfilemetadata_exiv2.cpp )

kde4_add_plugin(kfilemetadata_exiv2 ${kfilemetadata_exiv2_SRCS})
target_link_libraries(kfilemetadata_exiv2
    ${KDE4_KIO_LIBS}
    ${KDE4_KEXIV2_LIBS}
)

install(
    TARGETS kfilemetadata_exiv2
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(
    FILES kfilemetadata_exiv2.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
