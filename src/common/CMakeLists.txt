cmake_minimum_required(VERSION 3.5)
project(common)
add_library(${PROJECT_NAME} INTERFACE robin_hood.h types.h)
target_include_directories(${PROJECT_NAME} INTERFACE .)