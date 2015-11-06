# Create a symlink to executable, also sets the permission bits.

MESSAGE(STATUS "Symlinking $ENV{DESTDIR}/${LINK_NAME} to ${TARGET}")

GET_FILENAME_COMPONENT(abs_link_name $ENV{DESTDIR}/${LINK_NAME} ABSOLUTE)
GET_FILENAME_COMPONENT(link_path $ENV{DESTDIR}/${LINK_NAME} PATH)
GET_FILENAME_COMPONENT(abs_link_path ${link_path} ABSOLUTE)
MAKE_DIRECTORY(${abs_link_path})

GET_FILENAME_COMPONENT(abs_target ${TARGET} ABSOLUTE)
IF(UNIX)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink ${abs_target} ${abs_link_name})
    EXECUTE_PROCESS(COMMAND chmod a+x $ENV{DESTDIR}/${abs_target})
ELSE()
    MESSAGE(SEND_ERROR "Symlinking supported only on UNIX")
ENDIF()
