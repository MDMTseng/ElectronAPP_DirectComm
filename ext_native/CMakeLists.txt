cmake_minimum_required(VERSION 3.10)
project(dlib)

set(CMAKE_CXX_STANDARD 11)

add_library(dlib SHARED dlib.cpp)

set_target_properties(dlib PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "dlib"
)

# Set the output directory for the library
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})