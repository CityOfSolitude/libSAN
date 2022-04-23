cmake_minimum_required(VERSION 3.2)

project(SAN VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")

add_library(SAN src/san.cpp)
target_include_directories(SAN PUBLIC include)
target_include_directories(SAN PRIVATE src)

find_package(GTest QUIET)

if (NOT GTEST_FOUND)
    add_subdirectory(lib/googletest EXCLUDE_FROM_ALL)
endif ()

add_executable(unittest
        test/testTables.cpp
        test/testEncode24.cpp
        test/testEncode32.cpp
        test/testEncode48.cpp
        test/main.cpp)
target_include_directories(unittest PRIVATE src)
target_link_libraries(unittest gtest_main SAN)
add_test(NAME unittest COMMAND unittest)
