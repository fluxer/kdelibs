project(tetest)
 
find_package(KDELibs4 REQUIRED)

include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})
add_definitions(${QT_DEFINITIONS} ${KDE_DEFINITIONS})

########### next target ###############

set(hello_SRCS main.cc )
add_executable(tetest ${hello_SRCS})
target_link_libraries(tetest
    KDE4::kdecore
    KDE4::kdeui
    KDE4::kparts
)

########### install files ###############

install(
    TARGETS tetest
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)
