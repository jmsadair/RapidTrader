#include <gtest/gtest.h>
#include "order.h"

TEST(OrderTest, OrderAcceptsValidInput) {
    const auto order_action = OrderAction::Limit;
    const auto order_side = OrderSide::Ask;
    const auto order_type = OrderType::GoodTillCancel;
    const auto order_quantity = 25;
    const auto order_price = 30.0;
    const auto order_id = 123;
    const auto order_user_id = 1;

    ASSERT_NO_THROW(OrderBook::Order(order_action, order_side, order_type, order_quantity, order_price, order_id, order_user_id));
}

TEST(OrderTest, OrderRejectsNegativePrice) {
    const auto order_action = OrderAction::Limit;
    const auto order_side = OrderSide::Ask;
    const auto order_type = OrderType::FillOrKill;
    const auto order_quantity = 25;
    const auto order_price = -1.0;
    const auto order_id = 123;
    const auto order_user_id = 1;

    ASSERT_THROW(OrderBook::Order(order_action, order_side, order_type, order_quantity, order_price, order_id, order_user_id),
                 std::invalid_argument);
}

TEST(OrderTest, OrderRejectsZeroQuantity) {
    const auto order_action = OrderAction::Limit;
    const auto order_side = OrderSide::Bid;
    const auto order_type = OrderType::ImmediateOrCancel;
    const auto order_quantity = 0;
    const auto order_price = 13.0;
    const auto order_id = 123;
    const auto order_user_id = 1;

    ASSERT_THROW(OrderBook::Order(order_action, order_side, order_type, order_quantity, order_price, order_id, order_user_id),
                 std::invalid_argument);
}

TEST(OrderTest, DifferentOrdersNotEqual) {
    const auto order1_action = OrderAction::Limit;
    const auto order1_side = OrderSide::Bid;
    const auto order1_type = OrderType::GoodTillCancel;
    const auto order1_quantity = 301;
    const auto order1_price = 69.54;
    const auto order1_id = 123;
    const auto order1_user_id = 1;
    const auto order2_action = OrderAction::Limit;
    const auto order2_side = OrderSide::Bid;
    const auto order2_type = OrderType::GoodTillCancel;
    const auto order2_quantity = 301;
    const auto order2_price = 69.54;
    const auto order2_id = 2;
    const auto order2_user_id = 2;
    const auto order1 = OrderBook::Order(order1_action, order1_side, order1_type,
                        order1_quantity, order1_price, order1_id, order1_user_id);
    const auto order2 = OrderBook::Order(order2_action, order2_side, order2_type,
                         order2_quantity, order2_price, order2_id, order2_user_id);
    ASSERT_NE(order1, order2);
}

TEST(OrderTest, SameOrdersEqual) {
    const auto order1_action = OrderAction::Limit;
    const auto order1_side = OrderSide::Bid;
    const auto order1_type = OrderType::ImmediateOrCancel;
    const auto order1_quantity = 301;
    const auto order1_price = 69.54;
    const auto order1_id = 1;
    const auto order1_user_id = 1;
    const auto order2_action = OrderAction::Limit;
    const auto order2_side = OrderSide::Bid;
    const auto order2_type = OrderType::ImmediateOrCancel;
    const auto order2_quantity = 301;
    const auto order2_price = 69.54;
    const auto order2_id = 1;
    const auto order2_user_id = 1;
    const auto order1 = OrderBook::Order(order1_action, order1_side, order1_type,
                        order1_quantity, order1_price, order1_id, order1_user_id);
    const auto order2 = OrderBook::Order(order2_action, order2_side, order2_type,
                        order2_quantity, order2_price, order2_id, order2_user_id);

    ASSERT_EQ(order1, order2);
}

