cmake_minimum_required(VERSION 3.28)

project(ModuleTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(module_test main.cpp)

target_sources(module_test
    PRIVATE
        FILE_SET CXX_MODULES FILES
        math.cppm
)
