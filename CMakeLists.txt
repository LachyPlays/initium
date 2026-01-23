cmake_minimum_required(VERSION 3.21)
project(initium LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    set(CMAKE_MSVC_EXTERNAL_WARNINGS OFF)
endif()

find_package(Vulkan REQUIRED)

option(BUILD_SAMPLES "Build samples" ON)

add_subdirectory(src)

if (BUILD_SAMPLES)
    add_subdirectory(samples)
endif()


