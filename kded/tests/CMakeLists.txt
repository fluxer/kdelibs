include_directories( ${KDE4_KIO_INCLUDES} .. )

######### kmimeassociationstest ########

set(kmimeassociationstest_SRCS kmimeassociationstest.cpp ../kmimeassociations.cpp)

kde4_add_test(kmimeassociationstest ${kmimeassociationstest_SRCS})

target_link_libraries(kmimeassociationstest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

