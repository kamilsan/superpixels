cmake_minimum_required(VERSION 2.4)
project(SLIC)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(PROJECT_CODE
    include/centroid.hpp
    include/stb_image.h
    src/image.cpp include/image.hpp
    src/main.cpp)

include_directories(include)

add_executable(slic ${PROJECT_CODE})