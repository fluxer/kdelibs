set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${KDE4_KIO_INCLUDES})

set(krosstest_SRCS 
    testobject.cpp 
    main.cpp
)


add_executable(krosstest ${krosstest_SRCS})

target_link_libraries(krosstest ${KDE4_KDEUI_LIBS} krosscore )

