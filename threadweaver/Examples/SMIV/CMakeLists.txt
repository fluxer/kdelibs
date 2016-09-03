include_directories(
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/Weaver
    ${CMAKE_SOURCE_DIR}/Experimental
    ${KDE4_INCLUDES}
    ../../Weaver
    ../../Experimental
)

set(SMIV_SRCS
    SMIV.cpp
    SMIVItem.cpp
    SMIVModel.cpp
    SMIVView.cpp
    SMIVItemDelegate.cpp
    QImageLoaderJob.cpp
    ComputeThumbNailJob.cpp
)

add_executable(SMIV ${SMIV_SRCS})

target_link_libraries(SMIV
    ${KDE4_KDECORE_LIBS}
    ${KDE4_THREADWEAVER_LIBS}
    ${QT_QTGUI_LIBRARY}
    ThreadWeaverExperimental
)

install(TARGETS SMIV DESTINATION ${BIN_INSTALL_DIR})
