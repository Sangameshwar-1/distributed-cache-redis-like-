cmake_minimum_required(VERSION 3.10)
project(CacheX)
set(CMAKE_CXX_STANDARD 17)
include_directories(include)
add_executable(cachex src/main.cpp src/Cache.cpp src/Server.cpp)
if(WIN32)
    target_link_libraries(cachex ws2_32)
endif()


add_executable(benchmark benchmarks/benchmark.cpp src/Cache.cpp)

