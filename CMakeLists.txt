cmake_minimum_required(VERSION 3.2.0)
project(software_renderer VERSION 0.9.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(ersatz)
add_subdirectory(main)

add_dependencies(software_renderer ersatz)
