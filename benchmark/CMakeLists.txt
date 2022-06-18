cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.10.x
)

FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG master
)
set(BENCHMARK_ENABLE_TESTING off)

FetchContent_MakeAvailable(googletest googlebenchmark)

project(fast_exchange_benchmark LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

add_executable(${PROJECT_NAME} order_benchmark.cpp price_level_benchmark.cpp orderbook_benchmark.cpp exchange_bench.cpp)
target_link_libraries(${PROJECT_NAME} benchmark::benchmark fast_exchange)
