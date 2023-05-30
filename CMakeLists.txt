cmake_minimum_required(VERSION 3.0.0)
project(minilogger VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
endif(APPLE)

add_executable(minilogger main.cpp logger.hpp)
