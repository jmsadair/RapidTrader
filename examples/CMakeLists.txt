cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

project(fast_exchange_example LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

add_executable(${PROJECT_NAME} exchange_example.cpp)
target_link_libraries(${PROJECT_NAME} fast_exchange Threads::Threads)
