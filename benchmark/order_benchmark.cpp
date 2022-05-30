#include <benchmark/benchmark.h>
#include "../src/order_book/order.h"

static void BM_OrderCreation(benchmark::State& state) {
    const auto order_action = OrderAction::Limit;
    const auto order_side = OrderSide::Ask;
    const auto order_type = OrderType::GoodTillCancel;
    const auto order_quantity = 25;
    const auto order_price = 30;
    const auto order_id = 123;
    const auto order_user_id = 1;

    for (auto _ : state)
        benchmark::DoNotOptimize(Order(order_action, order_side, order_type, order_quantity,
                                       order_price, order_id, order_user_id));
}

BENCHMARK(BM_OrderCreation);

BENCHMARK_MAIN();