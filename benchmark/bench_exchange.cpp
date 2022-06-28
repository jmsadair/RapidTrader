#include <random>
#include <benchmark/benchmark.h>
#include <thread>
#include <atomic>
#include "exchange.h"
#include "command.h"

static std::atomic<int> counter{1};

std::vector<Command::PlaceOrder> generateOrderCommands(uint32_t num_commands, uint32_t num_symbols) {
    std::vector<Command::PlaceOrder> commands;
    commands.reserve(num_commands);
    uint32_t id = 0;
    uint32_t uid = 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<int> order_action_dist({60, 40});
    std::discrete_distribution<int> order_side_dist({55, 45});
    std::discrete_distribution<int> order_type_dist({70, 15, 15});
    std::uniform_int_distribution<uint32_t> price_dist{1, 500};
    std::uniform_int_distribution<uint64_t> quantity_dist{1, 20000};
    std::uniform_int_distribution<uint32_t> symbol_dist{1, num_symbols};
    std::vector<OrderAction> order_actions{OrderAction::Limit, OrderAction::Limit};
    std::vector<OrderSide> order_sides{OrderSide::Ask, OrderSide::Bid};
    std::vector<OrderType> order_types{OrderType::GoodTillCancel, OrderType::FillOrKill, OrderType::ImmediateOrCancel};

    for (uint32_t i = 0; i < num_commands; ++i) {
        OrderAction order_action = order_actions[order_action_dist(gen)];
        OrderSide order_side = order_sides[order_side_dist(gen)];
        OrderType order_type = order_types[order_type_dist(gen)];
        uint64_t order_quantity = quantity_dist(gen);
        uint32_t order_price = price_dist(gen);
        uint32_t symbol_id = symbol_dist(gen);
        commands.emplace_back(uid++, id++, symbol_id, order_price, order_quantity, order_action, order_side, order_type);
    }

    return commands;
}

void placeOrders(FastExchange::Exchange &exchange, std::vector<Command::PlaceOrder> &commands)
{
    FastExchange::ExchangeApi api = exchange.getApi();
    while (true)
    {
        auto current = ++counter;
        if (current >= commands.size())
        {
            return;
        }
        api.submitCommand(commands[current]);
    }
}

static void BM_Exchange(benchmark::State &state)
{
    auto commands = generateOrderCommands(3000000, 2500);
    for (auto _ : state)
    {
        state.PauseTiming();
        FastExchange::Exchange exchange;
        auto api = exchange.getApi();
        state.ResumeTiming();
        for (auto & command : commands)
            api.submitCommand(command);
    }
}

BENCHMARK(BM_Exchange)->Unit(benchmark::kMillisecond);
