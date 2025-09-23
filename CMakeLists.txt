cmake_minimum_required(VERSION 3.10)
project(Huffman_Coding VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES src/*.cpp)
list(APPEND SOURCES main.cpp)
add_executable(huff ${SOURCES})

target_include_directories(huff PRIVATE include)
