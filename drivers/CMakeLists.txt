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

TARGET_LINK_LIBRARIES(faux-driver m)

