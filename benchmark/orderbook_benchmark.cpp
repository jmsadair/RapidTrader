#include <random>
#include <benchmark/benchmark.h>
#include <thread>
#include "orderbook/vector_orderbook.h"


void placeOrder(OrderBook::VectorOrderBook &book, std::vector<Order> &orders, uint64_t numOrders) {
    for (uint64_t i = 0; i < numOrders; ++i) {
        book.placeOrder(orders[i]);
    }
}

static void BM_OrderBookPlaceOrder(benchmark::State& state) {
    uint64_t id = 0;
    const auto symbol = 1;
    const auto action = OrderAction::Limit;
    std::random_device seed;
    std::mt19937 gen{seed()};
    std::uniform_int_distribution price_dist{1, 100};
    std::uniform_int_distribution order_type_dist{0, 4};
    std::uniform_int_distribution order_side_dist{0, 1};
    std::uniform_int_distribution quantity_dist{1, 20000};
    std::vector<OrderSide> sides {OrderSide::Ask, OrderSide::Bid};
    std::vector<OrderType> types = {OrderType::GoodTillCancel, OrderType::GoodTillCancel, OrderType::GoodTillCancel,
                                    OrderType::GoodTillCancel, OrderType::GoodTillCancel};
    std::vector<Order> orders;
    Messaging::Receiver rec;
    orders.reserve(1000);
    for (auto _ : state) {
        state.PauseTiming();
        orders.clear();
        OrderBook::VectorOrderBook book(symbol, static_cast<Messaging::Sender>(rec));
        for (uint32_t i = 0; i < 1000; ++i) {
            orders.emplace_back(action, sides[order_side_dist(gen)], types[order_type_dist(gen)],
                                quantity_dist(gen), price_dist(gen), id, id++, symbol);
        }
        state.ResumeTiming();
        placeOrder(book, orders, 1000);
    }
}

BENCHMARK(BM_OrderBookPlaceOrder)->Unit(benchmark::kMillisecond);
