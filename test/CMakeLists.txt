cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
include(FetchContent)
enable_testing()
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.10.x
)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

project(fast_exchange_tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS --coverage)

add_executable(${PROJECT_NAME} order_test.cpp message_queue_test.cpp messaging_test.cpp orderbook_test.cpp)
target_link_libraries(${PROJECT_NAME} gtest_main fast_exchange)
install(TARGETS ${PROJECT_NAME} DESTINATION ${EXCHANGE_INSTALL_BIN_DIR})
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})