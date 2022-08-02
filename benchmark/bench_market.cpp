#include <random>
#include <benchmark/benchmark.h>
#include <iostream>
#include "market/market.h"
#include "market/concurrent_market.h"
#include "event_handler/event_handler.h"

class DebugEventHandler : public EventHandler
{};

void generateOrders(std::vector<Order> &orders, uint32_t num_orders, uint32_t num_symbols)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> uniform_dist{0, 9};
    std::uniform_int_distribution<uint64_t> symbol_dist{1, num_symbols};
    for (uint64_t i = 1; i <= num_orders; ++i)
    {
        OrderSide side = i % 2 == 0 ? OrderSide::Ask : OrderSide::Bid;
        OrderTimeInForce tof = OrderTimeInForce::GTC;
        uint64_t price_delta = side == OrderSide::Ask ? 1004 : 1000;
        uint64_t price = uniform_dist(gen) + price_delta;
        uint64_t quantity = (uniform_dist(gen) + 1) * 100;
        uint64_t order_id = i;
        uint64_t symbol_id = symbol_dist(gen);
        orders.push_back(side == OrderSide::Ask ? Order::limitAskOrder(order_id, symbol_id, price, quantity, tof)
                                                : Order::limitBidOrder(order_id, symbol_id, price, quantity, tof));
    }
}

static void BM_Market(benchmark::State &state)
{
    const uint64_t num_orders = 3500000;
    const uint64_t num_symbols = 1;
    std::vector<Order> orders;
    orders.reserve(num_orders);
    generateOrders(orders, num_orders, num_symbols);
    for (auto _ : state)
    {
        state.PauseTiming();
        RapidTrader::Matching::Market market{std::make_unique<EventHandler>()};
        for (int i = 1; i <= num_symbols; ++i)
            market.addSymbol(i, "MARKET BENCH");
        state.ResumeTiming();
        for (const auto &order : orders)
            market.addOrder(order);
    }
}

BENCHMARK(BM_Market)->Unit(benchmark::kMillisecond);
BENCHMARK_MAIN();