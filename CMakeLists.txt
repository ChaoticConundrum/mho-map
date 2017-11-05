## Mho-Map CMakeLists.txt
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(mhO-Map)

ADD_SUBDIRECTORY(libchaos)

ADD_SUBDIRECTORY(drivers)

### =================== SOURCES =================== ###

SET(SOURCES
    main.cpp
    mhodb.h
    mhodb.cpp
    tcpserver.h
    tcpserver.cpp
    connection.h
    netthread.h
    netthread.cpp
)

### =================== BUILD =================== ###

ADD_EXECUTABLE(mho-map ${SOURCES})
LibChaos_Configure_Target(mho-map)

