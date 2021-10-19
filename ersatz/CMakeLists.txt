cmake_minimum_required(VERSION 3.2.0)
project(ersatz VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ersatz 
	STATIC 
	src/common.cpp 
	src/allocators.cpp 
	include/ers/allocators.h
	include/ers/common.h
	include/ers/hash_map.h
	include/ers/linalg.h
	include/ers/macros.h
	include/ers/matrix.h
	include/ers/quaternion.h
	include/ers/typedefs.h
	include/ers/vec.h
	include/ers/vector.h	
	)

target_include_directories(ersatz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(ersatz PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
