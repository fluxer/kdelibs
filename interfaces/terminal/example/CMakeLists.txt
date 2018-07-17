project(tetest)
 
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})
add_definitions(${QT_DEFINITIONS} ${KDE_DEFINITIONS})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(hello_SRCS main.cc )
add_executable(tetest ${hello_SRCS})
target_link_libraries(tetest ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})


########### install files ###############
install(TARGETS tetest DESTINATION ${BIN_INSTALL_DIR})




