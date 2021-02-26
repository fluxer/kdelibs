include_directories(
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

set(ThreadWeaver_LIB_SRCS
    WeaverInterface.cpp
    ThreadWeaver.cpp
    WeaverImpl.cpp
    Thread.cpp
    Job.cpp
    State.cpp
    StateImplementation.cpp
    InConstructionState.cpp
    WorkingHardState.cpp
    SuspendingState.cpp
    SuspendedState.cpp
    ShuttingDownState.cpp
    DestructedState.cpp
    WeaverObserver.cpp
    JobCollection.cpp
    JobSequence.cpp
    DependencyPolicy.cpp
    ResourceRestrictionPolicy.cpp
)

add_library(threadweaver ${LIBRARY_TYPE} ${ThreadWeaver_LIB_SRCS})

target_link_libraries(threadweaver PUBLIC
    ${KDE4_KDECORE_LIBS}
)

set_target_properties(threadweaver PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(
    TARGETS threadweaver
    EXPORT kdelibsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(
    FILES
    WeaverInterface.h
    ThreadWeaver.h
    Thread.h
    Job.h
    State.h
    WeaverObserver.h
    JobCollection.h
    JobSequence.h
    DependencyPolicy.h
    ResourceRestrictionPolicy.h
    QueuePolicy.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/threadweaver
    COMPONENT Devel
)
