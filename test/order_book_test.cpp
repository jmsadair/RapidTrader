#include <gtest/gtest.h>
#include "order_book.h"

TEST(OrderBookTest, HandlesPlaceGtcOrderNoMatch) {
    // Create a new order book for the "AAPL" symbol.
    const auto symbol = "AAPL";
    OrderBook::OrderBook order_book(symbol);
    // Create some orders to add to the order book.
    const auto order1_action = OrderAction::Limit;
    const auto order1_side = OrderSide::Bid;
    const auto order1_type = OrderType::GoodTillCancel;
    const auto order1_quantity = 301;
    const auto order1_price = 69;
    const auto order1_id = 1;
    const auto order1_user_id = 1;
    OrderBook::Order order1(order1_action, order1_side, order1_type,
                 order1_quantity, order1_price, order1_id, order1_user_id);
    const auto order2_action = OrderAction::Limit;
    const auto order2_side = OrderSide::Bid;
    const auto order2_type = OrderType::GoodTillCancel;
    const auto order2_quantity = 245;
    const auto order2_price = 69;
    const auto order2_id = 2;
    const auto order2_user_id = 2;
    OrderBook::Order order2(order2_action, order2_side, order2_type,
                 order2_quantity, order2_price, order2_id, order2_user_id);
    const auto order3_action = OrderAction::Limit;
    const auto order3_side = OrderSide::Bid;
    const auto order3_type = OrderType::GoodTillCancel;
    const auto order3_quantity = 657;
    const auto order3_price = 220;
    const auto order3_id = 3;
    const auto order3_user_id = 3;
    OrderBook::Order order3(order3_action, order3_side, order3_type,
                 order3_quantity, order3_price, order3_id, order3_user_id);
    // Place the orders.
    order_book.placeOrder(order1);
    order_book.placeOrder(order2);
    order_book.placeOrder(order3);
    // Check if orders are in the order book.
    ASSERT_TRUE(order_book.hasOrder(order1));
    ASSERT_TRUE(order_book.hasOrder(order2));
    ASSERT_TRUE(order_book.hasOrder(order3));
    // None of the order should have been filled, as there are no matches.
    ASSERT_EQ(order1.status, OrderStatus::Accepted);
    ASSERT_EQ(order1.quantity, order1_quantity);
    ASSERT_EQ(order2.status, OrderStatus::Accepted);
    ASSERT_EQ(order2.quantity, order2_quantity);
    ASSERT_EQ(order3.status, OrderStatus::Accepted);
    ASSERT_EQ(order3.quantity, order3_quantity);
}

TEST(OrderBookTest, HandlesPlaceFokOrderWithMatch) {
    // Create a new order book for the "GOOGL" symbol.
    const auto symbol = "GOOGL";
    OrderBook::OrderBook order_book(symbol);
    // Create a GTC order that should be matched with the FOK order.
    const auto order1_action = OrderAction::Limit;
    const auto order1_side = OrderSide::Ask;
    const auto order1_type = OrderType::GoodTillCancel;
    const auto order1_quantity = 301;
    const auto order1_price = 69;
    const auto order1_id = 1;
    const auto order1_user_id = 1;
    OrderBook::Order order1(order1_action, order1_side, order1_type,
                 order1_quantity, order1_price, order1_id, order1_user_id);
    // Create a FOK order that will be matched.
    const auto order2_action = OrderAction::Limit;
    const auto order2_side = OrderSide::Bid;
    const auto order2_type = OrderType::FillOrKill;
    const auto order2_quantity = 245;
    const auto order2_price = 69;
    const auto order2_id = 2;
    const auto order2_user_id = 2;
    OrderBook::Order order2(order2_action, order2_side, order2_type,
                 order2_quantity, order2_price, order2_id, order2_user_id);
    // Place the orders.
    order_book.placeOrder(order1);
    order_book.placeOrder(order2);
    ASSERT_TRUE(order_book.hasOrder(order1));
    // FOK orders should never be added to the order book.
    ASSERT_FALSE(order_book.hasOrder(order2));
    // FOK order should have been completely filled.
    ASSERT_EQ(order2.status, OrderStatus::Filled);
    ASSERT_EQ(order2.quantity, 0);
    // GTC order should have been partially filled.
    ASSERT_EQ(order1.status, OrderStatus::PartiallyFilled);
    ASSERT_EQ(order1.quantity, order1_quantity - order2_quantity);
}

TEST(OrderBookTest, HandlesCancelOrder) {
    // Create a new order book for the "MSFT" symbol.
    const auto symbol = "MSFT";
    OrderBook::OrderBook order_book(symbol);
    // Create some orders to add to the order book.
    const auto order1_action = OrderAction::Limit;
    const auto order1_side = OrderSide::Ask;
    const auto order1_type = OrderType::GoodTillCancel;
    const auto order1_quantity = 5000;
    const auto order1_price = 233;
    const auto order1_id = 1;
    const auto order1_user_id = 1;
    OrderBook::Order order1(order1_action, order1_side, order1_type,
                 order1_quantity, order1_price, order1_id, order1_user_id);
    const auto order2_action = OrderAction::Limit;
    const auto order2_side = OrderSide::Ask;
    const auto order2_type = OrderType::GoodTillCancel;
    const auto order2_quantity = 780;
    const auto order2_price = 168;
    const auto order2_id = 2;
    const auto order2_user_id = 2;
    OrderBook::Order order2(order2_action, order2_side, order2_type,
                 order2_quantity, order2_price, order2_id, order2_user_id);
    // Add the orders to the order book.
    order_book.placeOrder(order1);
    order_book.placeOrder(order2);
    // Check if orders are in the order book.
    ASSERT_TRUE(order_book.hasOrder(order1));
    ASSERT_TRUE(order_book.hasOrder(order2));
    // Cancel an order
    order_book.cancelOrder(order1);
    ASSERT_FALSE(order_book.hasOrder(order1));
    ASSERT_EQ(order1.status, OrderStatus::Cancelled);
    ASSERT_EQ(order1.quantity, order1_quantity);
    // Cancel the other order.
    order_book.cancelOrder(order2);
    ASSERT_FALSE(order_book.hasOrder(order2));
    ASSERT_EQ(order2.status, OrderStatus::Cancelled);
    ASSERT_EQ(order2.quantity, order2_quantity);
}