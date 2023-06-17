include_directories( ${KDE4_KIO_INCLUDES} .. )

######### kmimeassociationstest ########

set(kmimeassociationstest_SRCS kmimeassociationstest.cpp ../kmimeassociations.cpp)

kde4_add_test(kded-kmimeassociationstest ${kmimeassociationstest_SRCS})

target_link_libraries(kded-kmimeassociationstest ${QT_QTTEST_LIBRARY} kdecore)

