cmake_minimum_required(VERSION 3.5)
project(order_book)
find_package(Boost)
if(Boost_FOUND)
    add_library(${PROJECT_NAME} INTERFACE map_orderbook.h order.h price_level.h order_book_router.h vector_orderbook.h
            orderbook_interface.h types.h)
    target_include_directories(${PROJECT_NAME} INTERFACE .)
    target_include_directories(${PROJECT_NAME} INTERFACE ${Boost_INCLUDE_DIRS})
endif()