#include <benchmark/benchmark.h>
#include "../src/order_book/order_list.h"

static void BM_OrderListCreationNoOrder(benchmark::State& state) {
    for (auto _ : state)
        benchmark::DoNotOptimize(OrderBook::OrderList());
}

static void BM_OrderListCreationWithOrder(benchmark::State& state) {
    const auto order_action = OrderAction::Limit;
    const auto order_side = OrderSide::Ask;
    const auto order_type = OrderType::GoodTillCancel;
    const auto order_quantity = 25;
    const auto order_price = 30;
    const auto order_id = 123;
    const auto order_user_id = 1;
    Order order { order_action, order_side, order_type, order_quantity, order_price, order_id, order_user_id };
    for (auto _ : state) {
        benchmark::DoNotOptimize(OrderBook::OrderList(order));
    }
}

static void BM_OrderListAddOrder(benchmark::State& state) {
    const auto order_action = OrderAction::Limit;
    const auto order_side = OrderSide::Ask;
    const auto order_type = OrderType::GoodTillCancel;
    const auto order_quantity = 25;
    const auto order_price = 30;
    const auto order_id = 123;
    const auto order_user_id = 1;
    Order order { order_action, order_side, order_type, order_quantity, order_price, order_id, order_user_id };
    for (auto _ : state) {
        OrderBook::OrderList order_list;
        benchmark::DoNotOptimize(order_list);
        order_list.addOrder(order);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_OrderListCreationNoOrder);
BENCHMARK(BM_OrderListCreationWithOrder);
BENCHMARK(BM_OrderListAddOrder);