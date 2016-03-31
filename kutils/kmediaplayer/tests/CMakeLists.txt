include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

foreach(mantest audio media)
    kde4_add_manual_test(kmediaplayer-${mantest}
        k${mantest}widgettest.cpp
    )
    target_link_libraries(kmediaplayer-${mantest} kmediaplayer)
endforeach()
