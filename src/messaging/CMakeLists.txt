cmake_minimum_required(VERSION 3.5)
project(messaging)
add_library(${PROJECT_NAME} INTERFACE message_queue.h sender.h template_dispatcher.h dispatcher.h receiver.h)
target_include_directories(${PROJECT_NAME} INTERFACE .)