cmake_minimum_required(VERSION 3.22)
project(kyoto LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-missing-braces")
set(CMAKE_C_FLAGS_DEBUG "-g -O2")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(CMAKE_C_STANDARD 23)

file(GLOB SRC_FILES src/*.c)

add_executable(kyoto WIN32 ${SRC_FILES})

find_package(assimp CONFIG REQUIRED)
target_link_libraries(kyoto PRIVATE assimp::assimp)
