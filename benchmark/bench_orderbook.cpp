#include <random>
#include <benchmark/benchmark.h>
#include <thread>
#include <iostream>
#include "orderbook/vector_orderbook.h"

void placeOrder(OrderBook::VectorOrderBook &book, std::vector<Order> &orders, uint64_t numOrders)
{
    for (uint64_t i = 0; i < numOrders; ++i)
    {
        book.placeOrder(orders[i]);
    }
}

static void BM_OrderBookPlaceOrder(benchmark::State &state)
{
    uint64_t id = 0;
    std::random_device seed;
    std::mt19937 gen{seed()};
    std::discrete_distribution<int> order_command_dist({80, 20});
    std::discrete_distribution<int> order_side_dist({50, 50});
    std::discrete_distribution<int> order_type_dist({70, 10, 20});
    std::uniform_int_distribution<uint32_t> price_dist{1, 100};
    std::uniform_int_distribution<uint64_t> quantity_dist{1, 20000};
    std::vector<OrderSide> order_sides{OrderSide::Ask, OrderSide::Bid};
    std::vector<OrderType> order_types{OrderType::GoodTillCancel, OrderType::FillOrKill, OrderType::ImmediateOrCancel};
    std::vector<Order> orders;
    Messaging::Receiver rec;
    orders.reserve(100);
    for (auto _ : state)
    {
        state.PauseTiming();
        uint32_t symbol = 1;
        orders.clear();
        OrderBook::VectorOrderBook book(symbol, static_cast<Messaging::Sender>(rec));
        benchmark::DoNotOptimize(book);
        for (uint32_t i = 0; i < 1000000; ++i)
        {
            OrderAction action = OrderAction::Limit;
            OrderSide side = order_sides[order_side_dist(gen)];
            OrderType type = order_types[order_type_dist(gen)];
            uint64_t quantity = quantity_dist(gen);
            uint32_t price = price_dist(gen);
            orders.emplace_back(action, side, type, quantity, price, id, id++, symbol);
        }
        state.ResumeTiming();
        placeOrder(book, orders, 1000000);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_OrderBookPlaceOrder)->Unit(benchmark::kMillisecond);
