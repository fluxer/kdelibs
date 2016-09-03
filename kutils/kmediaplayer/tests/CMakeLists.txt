include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

foreach(mantest media)
    kde4_add_manual_test(kmediaplayer-${mantest}
        k${mantest}widgettest.cpp
    )
    target_link_libraries(kmediaplayer-${mantest} ${KDE4_KMEDIAPLAYER_LIBS})
endforeach()
