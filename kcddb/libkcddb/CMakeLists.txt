########### next target ###############

set(kcddb_LIB_SRCS ${libmusicbrainz_SRCS}
    cache.cpp
    cdinfo.cpp
    kcddbconfig.cpp
    client.cpp
    kcddb.cpp
    cddb.cpp
    lookup.cpp
    cddbplookup.cpp
    synccddbplookup.cpp
    asynccddbplookup.cpp
    httplookup.cpp
    synchttplookup.cpp
    asynchttplookup.cpp
    smtpsubmit.cpp
    asyncsmtpsubmit.cpp
    syncsmtpsubmit.cpp
    submit.cpp
    sites.cpp
    httpsubmit.cpp
    asynchttpsubmit.cpp
    synchttpsubmit.cpp
    categories.cpp
    genres.cpp
    cdinfodialog.cpp
    cdinfoencodingwidget.cpp
)

kde4_add_kcfg_files(kcddb_LIB_SRCS configbase.kcfgc)

add_library(kcddb ${LIBRARY_TYPE} ${kcddb_LIB_SRCS})

target_link_libraries(kcddb ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
if(MUSICBRAINZ5_FOUND)
    target_link_libraries(kcddb ${MUSICBRAINZ5_LIBRARIES})
endif()

set_target_properties(kcddb PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

########### install files ###############

generate_export_header(kcddb)

install(
    FILES
    cdinfo.h
    cdinfodialog.h
    client.h
    genres.h
    kcddbconfig.h
    kcddb.h
    ${CMAKE_CURRENT_BINARY_DIR}/kcddb_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/configbase.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/libkcddb
    COMPONENT Devel
)

install(
    FILES libkcddb.kcfg
    DESTINATION ${KCFG_INSTALL_DIR}
)

install(
    TARGETS kcddb
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)
