set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )


########### fakehardwaretest ###############

set(fakehardwaretest_SRCS fakehardwaretest.cpp )


kde4_add_test(solid-fakehardwaretest ${fakehardwaretest_SRCS})

target_link_libraries(solid-fakehardwaretest solid_static ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} )
add_definitions(-DTEST_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw/fakecomputer.xml\\"")

########### solidhwtest ###############

set(solidhwtest_SRCS 
   solidhwtest.cpp )


kde4_add_test(solid-solidhwtest ${solidhwtest_SRCS})
add_definitions(-DFAKE_COMPUTER_XML="\\"${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw/fakecomputer.xml\\"")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw )

target_link_libraries(solid-solidhwtest ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${LIBS} solid_static)

########### solidnettestdbusservice ###############

#set(solidnettestdbusservice_SRCS
#   solidnettestdbusservice.cpp )


#QT4_ADD_DBUS_ADAPTOR(solidnettestdbusservice_SRCS ../solid/org.kde.Solid.Networking.Client.xml
#                         solidnettestdbusservice.h TestNetworkingService)

#add_executable(solidnettestdbusservice ${solidnettestdbusservice_SRCS})

#target_link_libraries(solidnettestdbusservice ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY})

########### networkingclient ###############

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} 
    ${KDE4_KDECORE_INCLUDES} ${KDE4_KDEUI_INCLUDES} ${KDE4_KIO_INCLUDES})

set(networkingclient_SRCS networkingclient.cpp networkingclientview.ui)

qt4_add_dbus_interface(networkingclient_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../solid/org.kde.Solid.Networking.Client.xml clientinterface)

add_executable(solid-networkingclient ${networkingclient_SRCS})

target_link_libraries(solid-networkingclient solid_static ${KDE4_KDEUI_LIBS})
########### install files ###############

