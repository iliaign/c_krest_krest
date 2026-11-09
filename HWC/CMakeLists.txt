cmake_minimum_required(VERSION 3.28)

project(DynamicSet LANGUAGES CXX)

add_executable(DynamicSet
    src/main.cpp
)

target_sources(DynamicSet
    PRIVATE
        FILE_SET CXX_MODULES
        BASE_DIRS sets tests
        FILES
            sets/dynamic_set.cppm
            sets/set_implementation.cppm
            sets/vector_implementation.cppm
            tests/test.cppm
)

# C++20 required only for our target
target_compile_features(DynamicSet PRIVATE cxx_std_20)

set_target_properties(DynamicSet PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
