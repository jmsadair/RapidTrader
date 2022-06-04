project(fast_exchange_benchmark)
find_package(benchmark REQUIRED)
add_executable(${PROJECT_NAME} order_benchmark.cpp price_level_benchmark.cpp orderbook_benchmark.cpp)
target_link_libraries(${PROJECT_NAME} orderbook common messaging messages benchmark::benchmark)
