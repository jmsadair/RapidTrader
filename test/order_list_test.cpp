#include <gtest/gtest.h>
#include "order_list.h"

TEST(OrderListTest, HandlesAddAndRemoveOrder) {
    const auto order1_action = OrderAction::Limit;
    const auto order1_side = OrderSide::Bid;
    const auto order1_type = OrderType::GoodTillCancel;
    const auto order1_quantity = 301;
    const auto order1_price = 695;
    const auto order1_id = 1;
    const auto order1_user_id = 1;
    const auto order2_action = OrderAction::Limit;
    const auto order2_side = OrderSide::Bid;
    const auto order2_type = OrderType::GoodTillCancel;
    const auto order2_quantity = 245;
    const auto order2_price = 695;
    const auto order2_id = 2;
    const auto order2_user_id = 2;
    const auto order3_action = OrderAction::Limit;
    const auto order3_side = OrderSide::Bid;
    const auto order3_type = OrderType::GoodTillCancel;
    const auto order3_quantity = 657;
    const auto order3_price = 220;
    const auto order3_id = 3;
    const auto order3_user_id = 3;
    const auto expected_size1 = 1;
    const auto expected_size2 = 2;
    const auto expected_size3 = 3;
    const auto expected_size4 = 0;
    auto order1 = OrderBook::Order(order1_action, order1_side, order1_type, order1_quantity,
                        order1_price, order1_id, order1_user_id);
    auto order2 = OrderBook::Order(order2_action, order2_side, order2_type, order2_quantity,
                        order2_price, order2_id, order2_user_id);
    auto order3 = OrderBook::Order(order3_action, order3_side, order3_type, order3_quantity,
                        order3_price, order3_id, order3_user_id);
    OrderBook::OrderList order_list;

    // Add the orders to the list.
    order_list.addOrder(order1);
    ASSERT_TRUE(order_list.size() == expected_size1);
    ASSERT_TRUE(order_list.hasOrder(order1));
    order_list.addOrder(order2);
    ASSERT_TRUE(order_list.size() == expected_size2);
    ASSERT_TRUE(order_list.hasOrder(order2));
    order_list.addOrder(order3);
    ASSERT_TRUE(order_list.size() == expected_size3);
    ASSERT_TRUE(order_list.hasOrder(order3));
    // Remove orders from the list.
    order_list.removeOrder(order1);
    ASSERT_TRUE(order_list.size() == expected_size2);
    ASSERT_TRUE(!order_list.hasOrder(order1));
    order_list.removeOrder(order2);
    ASSERT_TRUE(order_list.size() == expected_size1);
    ASSERT_TRUE(!order_list.hasOrder(order2));
    order_list.removeOrder(order3);
    ASSERT_TRUE(order_list.size() == expected_size4);
    ASSERT_TRUE(!order_list.hasOrder(order3));
}

TEST(OrderListTest, HandlesIsEmptyCorrectly) {
    const auto order_action = OrderAction::Limit;
    const auto order_side = OrderSide::Bid;
    const auto order_type = OrderType::GoodTillCancel;
    const auto order_quantity = 301;
    const auto order_price = 695;
    const auto order_id = 1;
    const auto order_user_id = 1;
    auto order = OrderBook::Order(order_action, order_side, order_type, order_quantity, order_price, order_id, order_user_id);
    auto order_list = OrderBook::OrderList(order);
    ASSERT_TRUE(!order_list.isEmpty());
    order_list.removeOrder(order);
    ASSERT_TRUE(order_list.isEmpty());
}