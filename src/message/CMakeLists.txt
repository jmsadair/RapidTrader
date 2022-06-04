cmake_minimum_required(VERSION 3.5)
project(messages)
add_library(${PROJECT_NAME} INTERFACE command.h event.h result.h)
target_include_directories(${PROJECT_NAME} INTERFACE .)
