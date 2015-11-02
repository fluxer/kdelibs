add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(modules)
add_subdirectory(console)
add_subdirectory(qts)

if(ENABLE_TESTING)
    add_subdirectory(test)
endif()
