#include <random>
#include <benchmark/benchmark.h>
#include <thread>
#include <atomic>
#include "exchange.h"

static std::atomic<int> counter{1};

void placeOrders(FastExchange::Exchange &exchange, uint32_t max_symbol_id, uint32_t num_commands)
{
    FastExchange::ExchangeApi api = exchange.getApi();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<int> order_command_dist({80, 20});
    std::discrete_distribution<int> order_side_dist({55, 45});
    std::discrete_distribution<int> order_type_dist({70, 15, 15});
    std::uniform_int_distribution<uint32_t> price_dist{1, 100};
    std::uniform_int_distribution<uint64_t> quantity_dist{1000, 20000};
    std::uniform_int_distribution<uint32_t> symbol_dist{1, max_symbol_id};
    std::vector<OrderSide> order_sides{OrderSide::Ask, OrderSide::Bid};
    std::vector<OrderType> order_types{OrderType::GoodTillCancel, OrderType::FillOrKill, OrderType::ImmediateOrCancel};
    while (true)
    {
        auto current = ++counter;
        if (current >= num_commands)
        {
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
        auto cmd =
            Message::Command::PlaceOrder(user_id, order_id, symbol_id, order_price, order_quantity, order_action, order_side, order_type);
        api.submitCommand(cmd);
    }
}

static void BM_Exchange(benchmark::State &state)
{
    int num_engines = 4;
    int num_event_handlers = 4;
    int num_threads = 3;
    uint32_t num_symbols = 5000;
    uint32_t num_commands = 3000000;
    for (auto _ : state)
    {
        state.PauseTiming();
        std::vector<std::thread> threads;
        threads.reserve(num_threads);
        counter.store(1);
        FastExchange::Exchange exchange(num_engines, num_event_handlers);
        exchange.start();
        auto api = exchange.getApi();
        for (uint32_t i = 0; i < num_symbols; ++i)
        {
            Message::Command::AddOrderBook add_book{i};
            api.submitCommand(add_book);
        }
        state.ResumeTiming();
        for (int i = 0; i < num_threads; ++i)
            threads.emplace_back([&] { placeOrders(exchange, num_symbols, num_commands); });
        for (int i = 0; i < num_threads; ++i)
            threads[i].join();
        threads.clear();
    }
}

BENCHMARK(BM_Exchange)->Unit(benchmark::kMillisecond);
