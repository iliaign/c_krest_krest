cmake_minimum_required(VERSION 3.28)
project(dynamic_set_hw LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Library with all data-structure modules (folder sets/) ---------------
add_library(sets)
target_sources(sets
    PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS sets
        FILES
            sets/dynamic_set.cppm
            sets/set_implementation.cppm
            sets/vector_implementation.cppm
            sets/avl_implementation.cppm
            # Uncomment after you create these two files:
            # sets/trie_implementation.cppm
            # sets/block_list_implementation.cppm
)

# --- Console application (folder src/) ------------------------------------
add_executable(set_app src/main.cpp)
target_link_libraries(set_app PRIVATE sets)

# --- Google Test (folder tests/) -------------------------------------------
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
# Needed on Windows so gtest uses the same runtime as the project.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(tests tests/tests.cpp)
target_link_libraries(tests PRIVATE sets GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)