cmake_minimum_required(VERSION 2.6)
project (monitoring)

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_subdirectory (collector)
add_subdirectory (daemon)
add_subdirectory (viewer)
