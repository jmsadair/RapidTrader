#include <random>
#include <benchmark/benchmark.h>
#include "vector_orderbook.h"

static void BM_OrderBookPlaceOrder(benchmark::State& state) {
    Messaging::Receiver receiver;
    OrderBook::VectorOrderBook book("MSFT", static_cast<Messaging::Sender>(receiver));
    OrderID id = 0;
    const auto symbol = "MSFT";
    const auto action = OrderAction::Limit;
    const auto type = OrderType::GoodTillCancel;
    std::random_device seed;
    std::mt19937 gen{seed()};
    std::uniform_int_distribution price_dist{50, 300};
    std::uniform_int_distribution order_side_dist{0, 1};
    std::uniform_int_distribution quantity_dist{1, 10000};
    std::vector<OrderSide> sides {OrderSide::Ask, OrderSide::Bid};
    std::vector<Message::Command::PlaceOrder> commands;
    commands.reserve(5000000);
    for (uint32_t i = 0; i < 5000000; ++i) {
        commands.emplace_back(id, id++, symbol, price_dist(gen), action, sides[order_side_dist(gen)], type, quantity_dist(gen));
    }
    auto it = commands.begin();
    for (auto _ : state) {
        book.placeOrder(*(it++));
    }
}

BENCHMARK(BM_OrderBookPlaceOrder);
