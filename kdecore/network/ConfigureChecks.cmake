####### checks for kdecore/network ###############

include(CMakePushCheckState)

cmake_reset_check_state()
set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${QT_INCLUDE_DIR}")
set(CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS} ${QT_DEFINITIONS}")
if (QT_USE_FRAMEWORKS)
    set(CMAKE_REQUIRED_FLAGS "-F${QT_LIBRARY_DIR} ")
endif (QT_USE_FRAMEWORKS)
check_cxx_source_compiles(
"#include <QtNetwork/QSslSocket>
int main()
{
    QSslSocket *socket;
    return 0;
}" HAVE_QSSLSOCKET
)

if (NOT HAVE_QSSLSOCKET)
    message(SEND_ERROR "KDE Requires Katie to be built with SSL support")
endif()
cmake_pop_check_state()
