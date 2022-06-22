cmake_minimum_required(VERSION 3.5)
find_package(Boost)
if (Boost_FOUND)
    include(FetchContent)
    FetchContent_Declare(
            spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.x
    )
    FetchContent_MakeAvailable(spdlog)

    project(fast_exchange LANGUAGES CXX)

    set(TARGET_SRC
            matching/matching_engine.cpp
            matching/order.cpp
            matching/orderbook/vector_orderbook.cpp
            #common/log.cpp
            event_handler.cpp
            exchange.cpp)

    set(TARGET_H
            common/types.h
            common/log.h
            common/robin_hood.h
            matching/matching_engine.h
            matching/matching_engine_router.h
            matching/order.h
            matching/symbol.h
            matching/orderbook/orderbook_interface.h
            matching/orderbook/vector_orderbook.h
            matching/orderbook/price_level.h
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
            matching
            common
            users
            .)
    target_link_libraries(${PROJECT_NAME} spdlog)
    target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
endif ()