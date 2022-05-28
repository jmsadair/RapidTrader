cmake_minimum_required(VERSION 3.5)
project(messaging)
add_library(messaging INTERFACE message_queue.h sender.h)
target_include_directories(messaging INTERFACE .)