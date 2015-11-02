include_directories(
    ${KDE4_KIO_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${CMAKE_SOURCE_DIR}/kio/httpfilter
)

kde4_add_test(kiohttp-httpheadertokenizetest httpheadertokenizetest.cpp)
target_link_libraries(kiohttp-httpheadertokenizetest
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

kde4_add_test(kiohttp-httpheaderdispositiontest httpheaderdispositiontest.cpp)
target_link_libraries(kiohttp-httpheaderdispositiontest
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

kde4_add_test(kiohttp-httpauthenticationtest httpauthenticationtest.cpp)
target_link_libraries(kiohttp-httpauthenticationtest
    ${KDE4_KDECORE_LIBS}
    kntlm
    ${QT_QTTEST_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)
if(GSSAPI_FOUND)
  target_link_libraries(kiohttp-httpauthenticationtest ${GSSAPI_LIBS})
endif()

kde4_add_test(kiohttp-httpobjecttest httpobjecttest.cpp
    ${kioslave-http_SOURCE_DIR}/http.cpp # requires solid
    ${kioslave-http_SOURCE_DIR}/httpauthentication.cpp
    ${httpfilter_STAT_SRCS}
    ${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc
)
target_link_libraries(kiohttp-httpobjecttest
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    kntlm
)
if(GSSAPI_FOUND)
    target_link_libraries(kiohttp-httpobjecttest ${GSSAPI_LIBS})
endif()
