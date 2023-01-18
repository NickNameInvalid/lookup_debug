cmake_minimum_required(VERSION 3.21)
project(Heap_Allocate_CPP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
include_directories(./include)
include_directories(./extern)
include_directories(./extern/CLUE/include)
include_directories(./extern/pisa/external/CLI11/include)
include_directories(./extern/pisa/tools)

set(PISA_BUILD_TOOLS OFF CACHE BOOL "skip building")
set(PISA_ENABLE_TESTING OFF CACHE BOOL "skip testing")
set(PISA_ENABLE_BENCHMARKING OFF CACHE BOOL "skip benchmarking")

add_subdirectory(extern/pisa EXCLUDE_FROM_ALL)

add_executable(Heap_Allocate_CPP main.cpp src/realtime_heap_allocate.cpp include/first_layer_index.hpp)
target_link_libraries(Heap_Allocate_CPP pisa)

