## Mho-Map CMakeLists.txt
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(JCache)

ADD_SUBDIRECTORY(libchaos)

ADD_SUBDIRECTORY(drivers)

### =================== SOURCES =================== ###

SET(SOURCES
    main.cpp
)

### =================== BUILD =================== ###

ADD_EXECUTABLE(mho-map ${SOURCES})
LibChaos_Configure_Target(mho-map)

