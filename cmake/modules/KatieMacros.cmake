# Copyright (c) 2015, Ivailo Monev, <xakepa10@gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.

if(NOT KATIE_UIC_EXECUTABLE)
    set(KATIE_UIC_EXECUTABLE "uic")
endif()
if(NOT KATIE_RCC_EXECUTABLE)
    set(KATIE_RCC_EXECUTABLE "rcc")
endif()
if(NOT KATIE_MOC_EXECUTABLE)
    set(KATIE_MOC_EXECUTABLE "moc")
endif()
if(NOT KATIE_DBUSXML2CPP_EXECUTABLE)
    set(KATIE_DBUSXML2CPP_EXECUTABLE "qdbusxml2cpp")
endif()
if(NOT KATIE_LRELEASE_EXECUTABLE)
    set(KATIE_LRELEASE_EXECUTABLE "lrelease")
endif()

macro(KATIE_RESOURCES RESOURCES)
    foreach(tmpres ${RESOURCES})
        get_filename_component(resource ${tmpres} ABSOLUTE)
        get_filename_component(rscext ${resource} EXT)
        get_filename_component(rscname ${resource} NAME_WE)
        if(${rscext} STREQUAL ".ui")
            set(rscout ${CMAKE_BINARY_DIR}/include/ui_${rscname}.h)
            add_custom_command(
                OUTPUT ${rscout}
                COMMAND ${KATIE_UIC_EXECUTABLE} "${resource}" -o "${rscout}"
                MAIN_DEPENDENCY "${resource}"
            )
        elseif(${rscext} STREQUAL ".qrc")
            set(rscout ${CMAKE_BINARY_DIR}/include/qrc_${rscname}.cpp)
            add_custom_command(
                OUTPUT ${rscout}
                COMMAND ${KATIE_RCC_EXECUTABLE} "${resource}" -o "${rscout}" -name "${rscname}"
                MAIN_DEPENDENCY ${resource}
            )
            set_property(SOURCE ${resource} APPEND PROPERTY OBJECT_DEPENDS ${rscout})
        elseif(${rscext} MATCHES "(.h|.cpp)")
            set(rscout ${CMAKE_BINARY_DIR}/include/moc_${rscname}${rscext})
            get_directory_property(dirdefs COMPILE_DEFINITIONS)
            get_directory_property(dirincs INCLUDE_DIRECTORIES)
            set(mocargs)
            foreach(ddef ${dirdefs})
                # TODO: filter non -D, support -U too
                set(mocargs ${mocargs} -D${ddef})
            endforeach()
            foreach(incdir ${dirincs})
                set(mocargs ${mocargs} -I${incdir})
            endforeach()
            add_custom_command(
                OUTPUT "${rscout}"
                COMMAND ${KATIE_MOC_EXECUTABLE} -nw "${resource}" -o "${rscout}" ${mocargs}
            )
            set_property(SOURCE ${resource} APPEND PROPERTY OBJECT_DEPENDS ${rscout})
        endif()
    endforeach()
endmacro()

macro(KATIE_DBUS_ADAPTOR SRCDEP SRCIN SRCOUT)
    if(${ARG4})
        set(dbusxmlargs ${ARG4})
    endif()
    get_filename_component(resource ${SRCIN} ABSOLUTE)
    set(rscout ${CMAKE_BINARY_DIR}/include/${SRCOUT}.h)
    set(mocout ${CMAKE_BINARY_DIR}/include/${SRCOUT}.moc)
    add_custom_command(
        OUTPUT "${rscout}"
        COMMAND "${KATIE_DBUSXML2CPP_EXECUTABLE}" -m "${resource}" -a "${rscout}" -p "${SRCOUT}" ${dbusxmlargs}
        COMMAND "${KATIE_MOC_EXECUTABLE}" -nw "${rscout}" -o "${mocout}" -i
    )
    set_property(SOURCE ${SRCDEP} APPEND PROPERTY OBJECT_DEPENDS ${rscout})
endmacro()

macro(KATIE_DBUS_INTERFACE SRCIN)
    if(${ARG4})
        set(dbusxmlargs ${ARG2})
    endif()
    string(REGEX MATCH ".*\\.(.*)\\.xml" ${SRCIN} SRCOUT)
    string(TOLOWER ${SRCIN} SRCIN)
    set(rscout ${CMAKE_BINARY_DIR}/include/${SRCOUT}ineterface.h)
    add_custom_command(
        OUTPUT "${rscout}"
        COMMAND "${KATIE_DBUSXML2CPP_EXECUTABLE}" -m "${SRCIN}" -a "${rscout}" -p "${SRCOUT}ineterface" ${dbusxmlargs}
    )
    set_property(SOURCE ${SRCIN} APPEND PROPERTY OBJECT_DEPENDS ${rscout})
endmacro()

# TODO: use add_custom_target()
macro(KATIE_TRANSLATION TRANSLATIONS)
    if(NOT QT_TRANSLATIONS_PATH_INST)
        message(SEND_ERROR "Directory where the translation should be installed is not set")
    endif()
    make_directory(${CMAKE_BINARY_DIR}/translations)
    foreach(translation ${TRANSLATIONS})
        get_filename_component(trname ${translation} NAME_WE)
        get_filename_component(trdir ${translation} DIRECTORY)
        set(trout ${trdir}/${trname}.qm)
        execute_process(
            COMMAND ${KATIE_LRELEASE_EXECUTABLE} ${translation}
        )
        install(FILES ${trout} DESTINATION ${QT_TRANSLATIONS_PATH_INST})
    endforeach()
endmacro() 
