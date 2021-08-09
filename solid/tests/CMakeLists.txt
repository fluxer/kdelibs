include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw
    ${CMAKE_CURRENT_BINARY_DIR}/../solid
)


########### fakehardwaretest ###############

kde4_add_test(solid-fakehardwaretest fakehardwaretest.cpp)

target_link_libraries(solid-fakehardwaretest
    ${KDE4_SOLID_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTTEST_LIBRARY}
)

add_definitions(-DTEST_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw/fakecomputer.xml\\"")

########### solidhwtest ###############

kde4_add_test(solid-solidhwtest solidhwtest.cpp)

add_definitions(-DFAKE_COMPUTER_XML="\\"${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw/fakecomputer.xml\\"")

target_link_libraries(solid-solidhwtest
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${KDE4_SOLID_LIBS}
)

########### solidnettestdbusservice ###############

#set(solidnettestdbusservice_SRCS
#   solidnettestdbusservice.cpp )


#QT4_ADD_DBUS_ADAPTOR(solidnettestdbusservice_SRCS ../solid/org.kde.Solid.Networking.Client.xml
#                         solidnettestdbusservice.h TestNetworkingService)

#kde4_add_manual_test(solidnettestdbusservice ${solidnettestdbusservice_SRCS})

#target_link_libraries(solidnettestdbusservice
# ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY})

########### networkingclient ###############

set(networkingclient_SRCS
    networkingclient.cpp
)

qt4_add_dbus_interface(networkingclient_SRCS
    ../solid/org.kde.Solid.Networking.Client.xml
    clientinterface
)

kde4_add_manual_test(solid-networkingclient ${networkingclient_SRCS})

target_link_libraries(solid-networkingclient ${KDE4_SOLID_LIBS} ${KDE4_KDEUI_LIBS})
