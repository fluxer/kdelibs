# the core library
add_subdirectory( core )
# the ui library
add_subdirectory( ui )
# optional modules
add_subdirectory( modules )
# commandline-prog to run scripts
add_subdirectory( console )

# unittests and such stuff
if(ENABLE_TESTING)
    add_subdirectory( test )
endif()

# QtScript backend
add_subdirectory( qts )
