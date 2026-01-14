cmake_minimum_required(VERSION 3.25)

set(APP_NAME huff)
set(SRC src)
set(INC include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(HuffmanCoding CXX)

add_executable(${APP_NAME}
  ${SRC}/huff.cpp
  ${SRC}/main.cpp
  ${SRC}/archive.cpp
  ${SRC}/bitio.cpp
  ${SRC}/trie.cpp
  ${SRC}/huffman_tree.cpp
)

target_include_directories(${APP_NAME} PRIVATE ${INC})

target_compile_options(${APP_NAME} PRIVATE
  -Wall
  -Wextra
  -Werror
  -Wpedantic
  -Wshadow
  -Wconversion
  -march=native
  -g
)
