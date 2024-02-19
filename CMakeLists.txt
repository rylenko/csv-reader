cmake_minimum_required(VERSION 3.14)
project(csv-reader)

# Compile flags
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -pedantic -Werror)
set(CMAKE_BUILD_TYPE Debug)

# Tests
include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/58d77fa8070e8cec2dc1ed015d66b454c8d78850.zip
)
FetchContent_MakeAvailable(googletest)
add_executable(test test.cpp)
target_link_libraries(test gtest_main)
