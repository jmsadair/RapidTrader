cmake_minimum_required(VERSION 3.5)
project(users)
add_library(${PROJECT_NAME} INTERFACE user.h user_router.h)
target_include_directories(${PROJECT_NAME} INTERFACE .)