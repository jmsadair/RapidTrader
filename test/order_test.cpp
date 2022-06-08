#include <gtest/gtest.h>
#include "order.h"

TEST(OrderTest, DifferentOrdersNotEqual) {
    const auto order1_type = OrderType::GoodTillCancel;
    const auto order1_quantity = 301;
    const auto order1_price = 69;
    const auto order1_id = 123;
    const auto order1_uid = 1;
    const auto order1_symbol_id = 1;
    const auto order2_type = OrderType::GoodTillCancel;
    const auto order2_quantity = 301;
    const auto order2_price = 69;
    const auto order2_id = 2;
    const auto order2_uid = 2;
    const auto order2_symbol_id = 1;
    const auto order1 = Order::bidLimit(order1_type, order1_quantity, order1_price, order1_id, order1_uid,
                                        order1_symbol_id);
    const auto order2 = Order::bidLimit(order2_type, order2_quantity, order2_price, order2_id, order2_uid,
                                        order2_symbol_id);
    ASSERT_NE(order1, order2);
}

TEST(OrderTest, SameOrdersEqual) {
    const auto order1_type = OrderType::ImmediateOrCancel;
    const auto order1_quantity = 301;
    const auto order1_price = 69;
    const auto order1_id = 1;
    const auto order1_uid = 1;
    const auto order1_symbol_id = 1;
    const auto order1 = Order::bidLimit(order1_type, order1_quantity, order1_price, order1_id, order1_uid,
                                        order1_symbol_id);
    const auto order2 = Order::bidLimit(order1_type, order1_quantity, order1_price, order1_id, order1_uid,
                                        order1_symbol_id);
    ASSERT_EQ(order1, order2);
}

