include_directories(
    ${KDE4_KPARTS_INCLUDES}
    ${KDE4_KJS_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/..

    ${CMAKE_SOURCE_DIR}/khtml/misc
    ${CMAKE_SOURCE_DIR}/khtml/dom
    ${CMAKE_SOURCE_DIR}/khtml/xml
    ${CMAKE_SOURCE_DIR}/khtml/html
    ${CMAKE_SOURCE_DIR}/khtml/rendering
    ${CMAKE_SOURCE_DIR}/khtml/ecma
    ${CMAKE_SOURCE_DIR}/khtml/imload
    ${CMAKE_SOURCE_DIR}/khtml/imload/decoders
    ${CMAKE_SOURCE_DIR}/khtml/java
    ${CMAKE_SOURCE_DIR}/khtml/svg
    ${CMAKE_SOURCE_DIR}/khtml/svg/graphics
    ${CMAKE_SOURCE_DIR}/khtml/platform/graphics
    ${CMAKE_SOURCE_DIR}/khtml/platform/text
    ${CMAKE_SOURCE_DIR}/khtml/platform
    ${CMAKE_SOURCE_DIR}/khtml/compat
    ${CMAKE_SOURCE_DIR}/khtml/css

    ${CMAKE_CURRENT_BINARY_DIR}/../ecma
    ${CMAKE_CURRENT_BINARY_DIR}/../java
    ${CMAKE_CURRENT_BINARY_DIR}/../css
)

kde4_add_test(khtml-parttest khtmlparttest.cpp)

target_link_libraries(khtml-parttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KHTML_LIBS}
)

########### next target ###############

set(khtmltest_SRCS
    testkhtmlnew.cpp
)

qt4_add_resources(khtmltest_SRCS testkhtml.qrc)

kde4_add_manual_test(khtml-test ${khtmltest_SRCS})

target_link_libraries(khtml-test
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KHTML_LIBS}
)
