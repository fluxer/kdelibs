project(threadweaver)

if(ENABLE_TESTING)
    add_subdirectory(Tests)
endif()

add_subdirectory(Weaver)
# disabled for default kdelibs build:
# add_subdirectory(WeaverGui)
# add_subdirectory(Experimental)
# add_subdirectory(Examples)

install(FILES threadweaver_export.h DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver COMPONENT Devel)
