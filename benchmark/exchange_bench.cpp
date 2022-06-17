#include <random>
#include <benchmark/benchmark.h>
#include <thread>
#include <atomic>
#include "exchange.h"
std::atomic<int> counter {1};

void placeOrders(ExchangeApi& api, uint32_t max_symbol_id, uint32_t num_commands) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<int> order_command_dist({80, 20});
    std::discrete_distribution<int> order_side_dist({60, 40});
    std::discrete_distribution<int> order_type_dist({70, 10, 10});
    std::uniform_int_distribution<uint32_t> price_dist {1, 500};
    std::uniform_int_distribution<uint64_t> quantity_dist {1, 20000};
    std::uniform_int_distribution<uint32_t> symbol_dist {1, max_symbol_id};
    std::vector<OrderSide> order_sides {OrderSide::Ask, OrderSide::Bid};
    std::vector<OrderType> order_types {OrderType::GoodTillCancel, OrderType::FillOrKill, OrderType::ImmediateOrCancel};
    while (true) {
        auto current = ++counter;
        if (current >= num_commands) {
            return;
        }
        OrderSide order_side = order_sides[order_side_dist(gen)];
        OrderType order_type = order_types[order_type_dist(gen)];
        OrderAction order_action = OrderAction::Limit;
        uint64_t order_quantity = quantity_dist(gen);
        uint32_t order_price = price_dist(gen);
        uint32_t order_id = current;
        uint32_t user_id = current;
        uint32_t symbol_id = symbol_dist(gen);
        auto cmd = Message::Command::PlaceOrder(user_id, order_id, symbol_id, order_price, order_quantity,
                                                order_action, order_side, order_type);
        api.submitCommand(cmd);
    }

}

static void BM_Exchange(benchmark::State& state) {
    size_t num_threads = 7;
    int num_engines = 4;
    uint32_t num_commands = 1000000;
    uint32_t num_symbols = 3000;
    std::vector<std::thread> threads;
    std::vector<ExchangeApi> apis;
    threads.reserve(num_threads);
    apis.reserve(num_threads);
    Exchange exchange(num_engines);
    for (int i = 0; i < num_threads; ++i)
        apis.push_back(exchange.getApi());
    for (uint32_t i = 0; i < num_symbols; ++i) {
        Message::Command::AddOrderBook add_book {i};
        apis[0].submitCommand(add_book);
    }
    for (auto _ : state) {
        for (std::size_t i = 0; i < num_threads - 1; i++) {
            threads.emplace_back([&] { placeOrders(apis[i], num_symbols, num_commands); });
        }
        // Wait for all threads to complete
        for (auto& t : threads) t.join();
        // Clear the threads each iteration of the benchmark
        threads.clear();
    }
}

BENCHMARK(BM_Exchange)->Unit(benchmark::kMillisecond);
