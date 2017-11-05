## Mho-Map drivers CMakeLists.txt
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(mhO-Map-drivers)

INCLUDE_DIRECTORIES(
    ..
)

SET(CMAKE_CXX_STANDARD 11)

ADD_LIBRARY(faux-driver MODULE
    faux-driver.cpp
)
LibChaos_Configure_Target(faux-driver)
#SET_TARGET_PROPERTIES(faux-driver PROPERTIES COMPILE_FLAGS "-fPIC")
TARGET_COMPILE_OPTIONS(faux-driver PRIVATE "-fPIC")
TARGET_LINK_LIBRARIES(faux-driver m)

