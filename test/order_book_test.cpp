#include <gtest/gtest.h>
#include "order_book.h"

// Places a GTC order with an empty order book.
TEST(OrderBookTest, HandlesPlaceGtcOrderNoMatch1) {
    const auto bid_order_action = OrderAction::Limit;
    const auto bid_order_side = OrderSide::Bid;
    const auto bid_order_type = OrderType::GoodTillCancel;
    const auto bid_order_quantity = 350;
    const auto bid_order_price = 100;
    const auto bid_order_id = 1;
    const auto bid_order_user_id = 1;
    Order bid_order(bid_order_action, bid_order_side, bid_order_type,
                     bid_order_quantity, bid_order_price, bid_order_id, bid_order_user_id);

    const auto symbol = "AAPL";
    OrderBook::OrderBook order_book(symbol);

    const auto expected_bid_order_status = OrderStatus::Accepted;

    order_book.placeOrder(bid_order);
    // The order book should now contain the order since there are no other orders.
    ASSERT_TRUE(order_book.hasOrder(bid_order));
    // Order should not have been filled.
    ASSERT_EQ(bid_order.quantity, bid_order_quantity);
    ASSERT_EQ(bid_order.status, expected_bid_order_status);
}

// Places only GTC ask orders.
TEST(OrderBookTest, HandlesPlaceGTCWithNoMatch2) {
    const auto ask_order1_action = OrderAction::Limit;
    const auto ask_order1_side = OrderSide::Ask;
    const auto ask_order1_type = OrderType::GoodTillCancel;
    const auto ask_order1_quantity = 98;
    const auto ask_order1_price = 100;
    const auto ask_order1_id = 9;
    const auto ask_order1_user_id = 9;
    Order ask_order1(ask_order1_action, ask_order1_side, ask_order1_type,
                     ask_order1_quantity, ask_order1_price, ask_order1_id, ask_order1_user_id);

    const auto ask_order2_action = OrderAction::Limit;
    const auto ask_order2_side = OrderSide::Ask;
    const auto ask_order2_type = OrderType::GoodTillCancel;
    const auto ask_order2_quantity = 60;
    const auto ask_order2_price = 100;
    const auto ask_order2_id = 10;
    const auto ask_order2_user_id = 10;
    Order ask_order2(ask_order2_action, ask_order2_side, ask_order2_type,
                     ask_order2_quantity, ask_order2_price, ask_order2_id, ask_order2_user_id);

    const auto symbol = "GOOGL";
    OrderBook::OrderBook order_book(symbol);

    const auto expected_ask_order1_status = OrderStatus::Accepted;
    const auto expected_ask_order2_status = OrderStatus::Accepted;

    order_book.placeOrder(ask_order1);
    order_book.placeOrder(ask_order2);
    // The order book should contain both orders since there are no bid orders to match with.
    ASSERT_TRUE(order_book.hasOrder(ask_order1));
    ASSERT_TRUE(order_book.hasOrder(ask_order2));
    // Orders should not have been filled.
    ASSERT_EQ(ask_order1.quantity, ask_order1_quantity);
    ASSERT_EQ(ask_order1.status, expected_ask_order1_status);
    ASSERT_EQ(ask_order2.quantity, ask_order2_quantity);
    ASSERT_EQ(ask_order2.status, expected_ask_order2_status);
}

// Places GTC ask and bid orders at unmatchable price levels.
TEST(OrderBookTest, HandlesPlaceGTCWithNoMatch3) {
    const auto ask_order_action = OrderAction::Limit;
    const auto ask_order_side = OrderSide::Ask;
    const auto ask_order_type = OrderType::GoodTillCancel;
    const auto ask_order_quantity = 780;
    const auto ask_order_price = 240;
    const auto ask_order_id = 1;
    const auto ask_order_user_id = 1;
    Order ask_order(ask_order_action, ask_order_side, ask_order_type,
                     ask_order_quantity, ask_order_price, ask_order_id, ask_order_user_id);

    const auto bid_order_action = OrderAction::Limit;
    const auto bid_order_side = OrderSide::Bid;
    const auto bid_order_type = OrderType::GoodTillCancel;
    const auto bid_order_quantity = 350;
    const auto bid_order_price = 100;
    const auto bid_order_id = 2;
    const auto bid_order_user_id = 2;
    Order bid_order(bid_order_action, bid_order_side, bid_order_type,
                    bid_order_quantity, bid_order_price, bid_order_id, bid_order_user_id);

    const auto symbol = "GOOGL";
    OrderBook::OrderBook order_book(symbol);

    const auto expected_ask_order_status = OrderStatus::Accepted;
    const auto expected_bid_order_status = OrderStatus::Accepted;

    // Bid and ask orders are placed, but they cannot be matched.
    order_book.placeOrder(bid_order);
    order_book.placeOrder(ask_order);
    // The order book should contain both orders since there are no bid orders to match with.
    ASSERT_TRUE(order_book.hasOrder(bid_order));
    ASSERT_TRUE(order_book.hasOrder(ask_order));
    // Orders should not have been filled.
    ASSERT_EQ(bid_order.quantity, bid_order_quantity);
    ASSERT_EQ(bid_order.status, expected_bid_order_status);
    ASSERT_EQ(ask_order.quantity, ask_order_quantity);
    ASSERT_EQ(ask_order.status, expected_ask_order_status);
}

// Places GTC ask and bid orders. Tests the case where the incoming order is matched only with
// orders from a single price level and is completely filled.
TEST(OrderBookTest, HandlesGtcOrderWithMatch1) {
    const auto ask_order1_action = OrderAction::Limit;
    const auto ask_order1_side = OrderSide::Ask;
    const auto ask_order1_type = OrderType::GoodTillCancel;
    const auto ask_order1_quantity = 98;
    const auto ask_order1_price = 100;
    const auto ask_order1_id = 1;
    const auto ask_order1_user_id = 1;
    Order ask_order1(ask_order1_action, ask_order1_side, ask_order1_type,
                     ask_order1_quantity, ask_order1_price, ask_order1_id, ask_order1_user_id);

    const auto ask_order2_action = OrderAction::Limit;
    const auto ask_order2_side = OrderSide::Ask;
    const auto ask_order2_type = OrderType::GoodTillCancel;
    const auto ask_order2_quantity = 60;
    const auto ask_order2_price = 100;
    const auto ask_order2_id = 2;
    const auto ask_order2_user_id = 2;
    Order ask_order2(ask_order2_action, ask_order2_side, ask_order2_type,
                     ask_order2_quantity, ask_order2_price, ask_order2_id, ask_order2_user_id);

    const auto bid_order1_action = OrderAction::Limit;
    const auto bid_order1_side = OrderSide::Bid;
    const auto bid_order1_type = OrderType::GoodTillCancel;
    const auto bid_order1_quantity = 60;
    const auto bid_order1_price = 120;
    const auto bid_order1_id = 3;
    const auto bid_order1_user_id = 3;
    Order bid_order1(bid_order1_action, bid_order1_side, bid_order1_type,
                    bid_order1_quantity, bid_order1_price, bid_order1_id, bid_order1_user_id);

    const auto symbol = "MSFT";
    OrderBook::OrderBook order_book(symbol);

    const auto expected_ask_order1_status = OrderStatus::Accepted;
    const auto expected_ask_order2_status = OrderStatus::Filled;
    const auto expected_bid_order1_status = OrderStatus::Filled;
    const auto expected_ask_order1_quantity = 98;
    const auto expected_ask_order2_quantity = 0;
    const auto expected_bid_order1_quantity = 0;

    // Ask orders are placed first. They are at the same price level.
    order_book.placeOrder(ask_order2);
    order_book.placeOrder(ask_order1);
    // The order book should contain both ask orders since there are no bid orders to match with.
    ASSERT_TRUE(order_book.hasOrder(ask_order1));
    ASSERT_TRUE(order_book.hasOrder(ask_order2));
    // Bid order is placed. This bid order can match with one of the ask orders that was previously placed.
    order_book.placeOrder(bid_order1);
    // Order should have never been inserted into the book since it was fully filled.
    ASSERT_FALSE(order_book.hasOrder(bid_order1));
    // This ask order should have been filled and removed from the book.
    ASSERT_FALSE(order_book.hasOrder(ask_order2));
    // This order should still be in the book.
    ASSERT_TRUE(order_book.hasOrder(ask_order1));
    // Check if orders have been correctly filled or not filled.
    ASSERT_EQ(bid_order1.quantity, expected_bid_order1_quantity);
    ASSERT_EQ(bid_order1.status, expected_bid_order1_status);
    ASSERT_EQ(ask_order1.quantity, expected_ask_order1_quantity);
    ASSERT_EQ(ask_order1.status, expected_ask_order1_status);
    ASSERT_EQ(ask_order2.quantity, expected_ask_order2_quantity);
    ASSERT_EQ(ask_order2.status, expected_ask_order2_status);
}

// Places GTC ask and bid orders. Tests the case where the incoming order is matched with
// orders from a multiple price levels and is completely filled.
TEST(OrderBookTest, HandlesGtcOrderWithMatch2) {
    const auto ask_order1_action = OrderAction::Limit;
    const auto ask_order1_side = OrderSide::Ask;
    const auto ask_order1_type = OrderType::GoodTillCancel;
    const auto ask_order1_quantity = 98;
    const auto ask_order1_price = 100;
    const auto ask_order1_id = 1;
    const auto ask_order1_user_id = 1;
    Order ask_order1(ask_order1_action, ask_order1_side, ask_order1_type,
                     ask_order1_quantity, ask_order1_price, ask_order1_id, ask_order1_user_id);

    const auto ask_order2_action = OrderAction::Limit;
    const auto ask_order2_side = OrderSide::Ask;
    const auto ask_order2_type = OrderType::GoodTillCancel;
    const auto ask_order2_quantity = 60;
    const auto ask_order2_price = 105;
    const auto ask_order2_id = 2;
    const auto ask_order2_user_id = 2;
    Order ask_order2(ask_order2_action, ask_order2_side, ask_order2_type,
                     ask_order2_quantity, ask_order2_price, ask_order2_id, ask_order2_user_id);

    const auto ask_order3_action = OrderAction::Limit;
    const auto ask_order3_side = OrderSide::Ask;
    const auto ask_order3_type = OrderType::GoodTillCancel;
    const auto ask_order3_quantity = 100;
    const auto ask_order3_price = 110;
    const auto ask_order3_id = 3;
    const auto ask_order3_user_id = 3;
    Order ask_order3(ask_order3_action, ask_order3_side, ask_order3_type,
                     ask_order3_quantity, ask_order3_price, ask_order3_id, ask_order3_user_id);

    const auto bid_order1_action = OrderAction::Limit;
    const auto bid_order1_side = OrderSide::Bid;
    const auto bid_order1_type = OrderType::GoodTillCancel;
    const auto bid_order1_quantity = 200;
    const auto bid_order1_price = 110;
    const auto bid_order1_id = 4;
    const auto bid_order1_user_id = 4;
    Order bid_order1(bid_order1_action, bid_order1_side, bid_order1_type,
                     bid_order1_quantity, bid_order1_price, bid_order1_id, bid_order1_user_id);

    const auto symbol = "MSFT";
    OrderBook::OrderBook order_book(symbol);

    const auto expected_ask_order1_status = OrderStatus::Filled;
    const auto expected_ask_order2_status = OrderStatus::Filled;
    const auto expected_ask_order3_status = OrderStatus::PartiallyFilled;
    const auto expected_bid_order1_status = OrderStatus::Filled;
    const auto expected_ask_order1_quantity = 0;
    const auto expected_ask_order2_quantity = 0;
    const auto expected_ask_order3_quantity = ask_order3_quantity - (bid_order1_quantity - ask_order1_quantity - ask_order2_quantity);
    const auto expected_bid_order1_quantity = 0;

    // Ask orders are placed first. They are at different price levels.
    order_book.placeOrder(ask_order1);
    order_book.placeOrder(ask_order2);
    order_book.placeOrder(ask_order3);
    // The order book should contain both ask orders since there are no bid orders to match with.
    ASSERT_TRUE(order_book.hasOrder(ask_order1));
    ASSERT_TRUE(order_book.hasOrder(ask_order2));
    ASSERT_TRUE(order_book.hasOrder(ask_order3));
    // Bid order is placed. This bid order can match with any of the previous ask orders.
    order_book.placeOrder(bid_order1);
    // Incoming order should have never been inserted into the book since it was fully filled.
    ASSERT_FALSE(order_book.hasOrder(bid_order1));
    // These orders should have been filled and removed from the book.
    ASSERT_FALSE(order_book.hasOrder(ask_order1));
    ASSERT_FALSE(order_book.hasOrder(ask_order2));
    // This order should still be in the book since it was not completely filled.
    ASSERT_TRUE(order_book.hasOrder(ask_order3));
    // Check if orders have been correctly filled or not filled.
    ASSERT_EQ(bid_order1.quantity, expected_bid_order1_quantity);
    ASSERT_EQ(bid_order1.status, expected_bid_order1_status);
    ASSERT_EQ(ask_order1.quantity, expected_ask_order1_quantity);
    ASSERT_EQ(ask_order1.status, expected_ask_order1_status);
    ASSERT_EQ(ask_order2.quantity, expected_ask_order2_quantity);
    ASSERT_EQ(ask_order2.status, expected_ask_order2_status);
    ASSERT_EQ(ask_order3.quantity, expected_ask_order3_quantity);
    ASSERT_EQ(ask_order3.status, expected_ask_order3_status);
}

// Places GTC ask and bid orders. Tests the case where the incoming order is only partially filled
// and is inserted into the order book.
TEST(OrderBookTest, HandlesPlaceGTCWithMatch3) {
    const auto ask_order_action = OrderAction::Limit;
    const auto ask_order_side = OrderSide::Ask;
    const auto ask_order_type = OrderType::GoodTillCancel;
    const auto ask_order_quantity = 780;
    const auto ask_order_price = 240;
    const auto ask_order_id = 1;
    const auto ask_order_user_id = 1;
    Order ask_order(ask_order_action, ask_order_side, ask_order_type,
                    ask_order_quantity, ask_order_price, ask_order_id, ask_order_user_id);

    const auto bid_order_action = OrderAction::Limit;
    const auto bid_order_side = OrderSide::Bid;
    const auto bid_order_type = OrderType::GoodTillCancel;
    const auto bid_order_quantity = 1000;
    const auto bid_order_price = 300;
    const auto bid_order_id = 2;
    const auto bid_order_user_id = 2;
    Order bid_order(bid_order_action, bid_order_side, bid_order_type,
                    bid_order_quantity, bid_order_price, bid_order_id, bid_order_user_id);

    const auto symbol = "GOOGL";
    OrderBook::OrderBook order_book(symbol);

    const auto expected_ask_order_status = OrderStatus::Filled;
    const auto expected_bid_order_status = OrderStatus::PartiallyFilled;
    const auto expected_ask_order_quantity = 0;
    const auto expected_bid_order_quantity = bid_order_quantity - ask_order_quantity;

    // Bid and ask orders are placed, but they cannot be matched.
    order_book.placeOrder(ask_order);
    order_book.placeOrder(bid_order);
    // The order book should contain this bid order since it has not been completely filled yet.
    ASSERT_TRUE(order_book.hasOrder(bid_order));
    // This order has been filled and should have been removed from the book.
    ASSERT_FALSE(order_book.hasOrder(ask_order));
    // Check if orders have been correctly filled or not filled.
    ASSERT_EQ(bid_order.quantity, expected_bid_order_quantity);
    ASSERT_EQ(bid_order.status, expected_bid_order_status);
    ASSERT_EQ(ask_order.quantity, expected_ask_order_quantity);
    ASSERT_EQ(ask_order.status, expected_ask_order_status);
}