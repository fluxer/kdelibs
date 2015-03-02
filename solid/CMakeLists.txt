include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${QT_INCLUDE_DIR}
                     ${QT_QTCORE_INCLUDE_DIR} )

add_subdirectory( solid )
add_subdirectory( tests )
