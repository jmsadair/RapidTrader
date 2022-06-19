cmake_minimum_required(VERSION 3.5)
include(FetchContent)

FetchContent_Declare(
        intrusive
        GIT_REPOSITORY https://github.com/boostorg/intrusive.git
        GIT_TAG master
)

FetchContent_MakeAvailable(intrusive)

project(fast_exchange LANGUAGES CXX)

set(TARGET_SRC
        matching_engine/matching_engine.cpp
        matching_engine/order.cpp
        matching_engine/vector_orderbook.cpp)

set(TARGET_H
        common/robin_hood.h
        common/types.h
        matching_engine/matching_engine.h
        matching_engine/matching_engine_router.h
        matching_engine/order.h
        matching_engine/orderbook_interface.h
        matching_engine/symbol.h
        matching_engine/vector_orderbook.h
        message/command.h
        message/event.h
        message/result.h
        messaging/dispatcher.h
        messaging/message_queue.h
        messaging/receiver.h
        messaging/sender.h
        messaging/template_dispatcher.h
        users/user.h
        users/user_router.h
        event_handler.h
        exchange.h
        exchange_api.h)

add_library(${PROJECT_NAME} ${TARGET_SRC} ${TARGET_H})
target_include_directories(${PROJECT_NAME} PUBLIC
        ${Boost_INCLUDE_DIRS}
        messaging
        message
        matching_engine
        common
        users
        .)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
