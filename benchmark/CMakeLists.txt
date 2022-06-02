project(fast_exchange_benchmark)
find_package(benchmark REQUIRED)
add_executable(${PROJECT_NAME} order_benchmark.cpp price_level_benchmark.cpp)
target_link_libraries(${PROJECT_NAME} order_book messaging messages benchmark::benchmark)
