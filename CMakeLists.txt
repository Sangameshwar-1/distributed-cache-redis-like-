cmake_minimum_required(VERSION 3.10)
project(CacheX)
set(CMAKE_CXX_STANDARD 17)
include_directories(include)
add_executable(cachex src/main.cpp src/Cache.cpp)

