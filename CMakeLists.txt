cmake_minimum_required(VERSION 3.10)
project(CacheX)
set(CMAKE_CXX_STANDARD 17)
include_directories(include)

# AddressSanitizer option
option(ASAN "Enable AddressSanitizer" OFF)
if(ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif()

# Main executable
add_executable(cachex src/main.cpp src/Cache.cpp src/Server.cpp src/Logger.cpp src/Config.cpp)
if(WIN32)
    target_link_libraries(cachex ws2_32)
endif()

# Benchmark
add_executable(benchmark benchmarks/benchmark.cpp src/Cache.cpp)

# Unit tests
add_executable(test_cache
    tests/test_cache.cpp
    tests/test_ttl.cpp
    tests/test_lru.cpp
    tests/test_persistence.cpp
    tests/test_hash.cpp
    src/Cache.cpp
)

# CTest integration
enable_testing()
add_test(NAME UnitTests COMMAND test_cache)
add_test(NAME Benchmark COMMAND benchmark)
