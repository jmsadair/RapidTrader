project(fast_exchange_benchmark)
find_package(benchmark REQUIRED)
add_executable(${PROJECT_NAME} order_benchmark.cpp)
target_link_libraries(${PROJECT_NAME} order_book benchmark::benchmark)
