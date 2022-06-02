#include <benchmark/benchmark.h>
#include "price_level.h"

static void BM_PriceLevelCreationNoOrder(benchmark::State& state) {
    for (auto _ : state)
        benchmark::DoNotOptimize(OrderBook::PriceLevel());
}

static void BM_PriceLevelCreationWithOrder(benchmark::State& state) {
    const auto order_action = OrderAction::Limit;
    const auto order_side = OrderSide::Ask;
    const auto order_type = OrderType::GoodTillCancel;
    const auto order_quantity = 25;
    const auto order_price = 30;
    const auto order_id = 123;
    const auto order_user_id = 1;
    Order order { order_action, order_side, order_type, order_quantity, order_price, order_id, order_user_id };
    for (auto _ : state) {
        benchmark::DoNotOptimize(OrderBook::PriceLevel(order));
    }
}

static void BM_PriceLevelAddOrder(benchmark::State& state) {
    const auto order_action = OrderAction::Limit;
    const auto order_side = OrderSide::Ask;
    const auto order_type = OrderType::GoodTillCancel;
    const auto order_quantity = 25;
    const auto order_price = 30;
    const auto order_id = 123;
    const auto order_user_id = 1;
    Order order { order_action, order_side, order_type, order_quantity, order_price, order_id, order_user_id };
    for (auto _ : state) {
        OrderBook::PriceLevel price_level;
        benchmark::DoNotOptimize(price_level);
        price_level.order_list.push_back(order);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_PriceLevelCreationNoOrder);
BENCHMARK(BM_PriceLevelCreationWithOrder);
BENCHMARK(BM_PriceLevelAddOrder);