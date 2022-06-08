#include <random>
#include <benchmark/benchmark.h>
#include <thread>
#include "vector_orderbook.h"
#include "exchange.h"

void placeOrder(OrderBook::VectorOrderBook &book, std::vector<Message::Command::PlaceOrder> &commands,
                uint64_t numCommands) {
    for (int i = 0; i < numCommands; ++i) {
        book.placeOrder(commands[i]);
    }
}

static void BM_OrderBookPlaceOrder(benchmark::State& state) {
    OrderID id = 0;
    const auto symbol = "MSFT";
    const auto action = OrderAction::Limit;
    const auto type = OrderType::GoodTillCancel;
    std::random_device seed;
    std::mt19937 gen{seed()};
    std::uniform_int_distribution price_dist{1, 100};
    std::uniform_int_distribution order_side_dist{0, 1};
    std::uniform_int_distribution quantity_dist{1, 20000};
    std::vector<OrderSide> sides {OrderSide::Ask, OrderSide::Bid};
    std::vector<Message::Command::PlaceOrder> commands;
    Messaging::Receiver rec;
    commands.reserve(4000000);
    for (auto _ : state) {
        state.PauseTiming();
        commands.clear();
        OrderBook::VectorOrderBook book(symbol, static_cast<Messaging::Sender>(rec));
        for (uint32_t i = 0; i < 4000000; ++i) {
            commands.emplace_back(id, id++, symbol, price_dist(gen), action, sides[order_side_dist(gen)],
                                  type, quantity_dist(gen));
        }
        state.ResumeTiming();
        placeOrder(book, commands, 4000000);
    }
}

static void BM_ExchangeOrder(benchmark::State& state) {
    OrderID id = 0;
    const auto symbol = "MSFT";
    const auto action = OrderAction::Limit;
    const auto type = OrderType::GoodTillCancel;
    std::random_device seed;
    std::mt19937 gen{seed()};
    std::uniform_int_distribution price_dist{1, 100};
    std::uniform_int_distribution order_side_dist{0, 1};
    std::uniform_int_distribution quantity_dist{1, 20000};
    std::vector<OrderSide> sides {OrderSide::Ask, OrderSide::Bid};
    std::vector<Message::Command::PlaceOrder> commands1;
    std::vector<Message::Command::PlaceOrder> commands2;
    std::vector<Message::Command::PlaceOrder> commands3;
    Messaging::Receiver rec;
    commands.reserve(4000000);
    for (auto _ : state) {
        state.PauseTiming();
        commands.clear();
        OrderBook::VectorOrderBook book(symbol, static_cast<Messaging::Sender>(rec));
        for (uint32_t i = 0; i < 4000000; ++i) {
            commands.emplace_back(id, id++, symbol, price_dist(gen), action, sides[order_side_dist(gen)],
                                  type, quantity_dist(gen));
        }
        state.ResumeTiming();
        placeOrder(book, commands, 4000000);
    }
}

BENCHMARK(BM_OrderBookPlaceOrder)->Unit(benchmark::kMillisecond);
