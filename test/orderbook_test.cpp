#include <gtest/gtest.h>
#include <thread>
#include "vector_orderbook.h"

struct OrderBookReceiver {
    Messaging::Receiver receiver;
    std::vector<Message::Event::TradeEvent> trade_events {};
    std::vector<Message::Event::OrderExecuted> orders_executed {};
    void start() {
        try {
            while (true) {
                receiver.wait()
                        .handle<Message::Event::TradeEvent>([&](const Message::Event::TradeEvent& msg) {
                            trade_events.push_back(msg);
                        })
                        .handle<Message::Event::OrderExecuted>([&](const Message::Event::OrderExecuted& msg) {
                            orders_executed.push_back(msg);
                        });
            }
        } catch(const Messaging::CloseQueue&) {}
    }
    void stop() {
        auto sender = static_cast<Messaging::Sender>(receiver);
        sender.send(Messaging::CloseQueue());
    }
};

TEST(VectorOrderBookTest, BookShouldInsertUnmatchedOrders1) {
    // Create a command to place an order.
    const auto uid = 1;
    const auto id = 1;
    const auto price = 100;
    const auto symbol = "MSFT";
    const auto action = OrderAction::Limit;
    const auto side = OrderSide::Ask;
    const auto type = OrderType::GoodTillCancel;
    const auto quantity = 200;
    Message::Command::PlaceOrder command {uid, id, symbol, price, action, side, type, quantity};
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, std::ref(result_receiver)};
    // Create the order book.
    OrderBook::VectorOrderBook book {"MSFT", static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place a new order.
    book.placeOrder(command);
    result_receiver.stop();
    t1.join();
    // Book should now contain the order since it is a GTC order and has no match.
    ASSERT_TRUE(book.hasOrder(command.order_id));
    // Get the order and check that it is unmodified.
    const auto& order = book.getOrder(command.order_id);
    ASSERT_EQ(order.status, OrderStatus::Accepted);
    ASSERT_EQ(order.quantity, quantity);
    // Order should not have been traded, so no messages should have been sent.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

TEST(VectorOrderBookTest, BookShouldInsertUnmatchedOrders2) {
    // Create commands to place orders on same side of book.
    const auto uid1 = 1;
    const auto id1 = 1;
    const auto price1 = 100;
    const auto symbol1 = "MSFT";
    const auto action1 = OrderAction::Limit;
    const auto side1 = OrderSide::Ask;
    const auto type1 = OrderType::GoodTillCancel;
    const auto quantity1 = 200;
    Message::Command::PlaceOrder command1 {uid1, id1, symbol1, price1, action1, side1, type1, quantity1};
    const auto uid2 = 2;
    const auto id2 = 2;
    const auto price2 = 200;
    const auto symbol2 = "MSFT";
    const auto action2 = OrderAction::Limit;
    const auto side2 = OrderSide::Ask;
    const auto type2 = OrderType::GoodTillCancel;
    const auto quantity2 = 120;
    Message::Command::PlaceOrder command2 {uid2, id2, symbol2, price2, action2, side2, type2, quantity2};
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {symbol1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new orders.
    book.placeOrder(command1);
    book.placeOrder(command2);
    result_receiver.stop();
    t1.join();
    // Book should now contain both orders since they are GTC orders that have no match.
    // Orders on the same side of the book should never match with one another.
    ASSERT_TRUE(book.hasOrder(command1.order_id));
    ASSERT_TRUE(book.hasOrder(command2.order_id));
    // Get the first order and check that it is unmodified.
    const auto& order1 = book.getOrder(command1.order_id);
    ASSERT_EQ(order1.status, OrderStatus::Accepted);
    ASSERT_EQ(order1.quantity, quantity1);
    ASSERT_EQ(order1.quantity_to_fill, order1.quantity);
    // Get the second order and check that it is unmodified.
    const auto& order2 = book.getOrder(command2.order_id);
    ASSERT_EQ(order2.status, OrderStatus::Accepted);
    ASSERT_EQ(order2.quantity, quantity2);
    ASSERT_EQ(order2.quantity_to_fill, order2.quantity);
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

TEST(VectorOrderBookTest, BookShouldInsertUnmatchedOrders3) {
    // Create commands to place orders on different sides of the book, but unmatchable price levels.
    const auto uid1 = 1;
    const auto id1 = 1;
    const auto price1 = 100;
    const auto symbol1 = "MSFT";
    const auto action1 = OrderAction::Limit;
    const auto side1 = OrderSide::Ask;
    const auto type1 = OrderType::GoodTillCancel;
    const auto quantity1 = 200;
    Message::Command::PlaceOrder command1 {uid1, id1, symbol1, price1, action1, side1, type1, quantity1};
    const auto uid2 = 2;
    const auto id2 = 2;
    const auto price2 = 50;
    const auto symbol2 = "MSFT";
    const auto action2 = OrderAction::Limit;
    const auto side2 = OrderSide::Ask;
    const auto type2 = OrderType::GoodTillCancel;
    const auto quantity2 = 120;
    Message::Command::PlaceOrder command2 {uid2, id2, symbol2, price2, action2, side2, type2, quantity2};
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {symbol1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new orders.
    book.placeOrder(command1);
    book.placeOrder(command2);
    result_receiver.stop();
    t1.join();
    // Book should now contain both orders since they are GTC orders that have no match.
    // Orders on the same side of the book should never match with one another.
    ASSERT_TRUE(book.hasOrder(command1.order_id));
    ASSERT_TRUE(book.hasOrder(command2.order_id));
    // Get the first order and check that it is unmodified.
    const auto& order1 = book.getOrder(command1.order_id);
    ASSERT_EQ(order1.status, OrderStatus::Accepted);
    ASSERT_EQ(order1.quantity_to_fill, order1.quantity);
    // Get the second order and check that it is unmodified.
    const auto& order2 = book.getOrder(command2.order_id);
    ASSERT_EQ(order2.status, OrderStatus::Accepted);
    ASSERT_EQ(order2.quantity_to_fill, order2.quantity);
    // Check the receiver for result messages.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

TEST(VectorOrderBookTest, BookShouldMatchOrders1) {
    // Create commands to place orders on different sides of the book, with matchable price levels.
    const auto uid1 = 1;
    const auto id1 = 1;
    const auto price1 = 100;
    const auto symbol1 = "MSFT";
    const auto action1 = OrderAction::Limit;
    const auto side1 = OrderSide::Ask;
    const auto type1 = OrderType::GoodTillCancel;
    const auto quantity1 = 100;
    Message::Command::PlaceOrder command1 {uid1, id1, symbol1, price1, action1, side1, type1, quantity1};
    const auto uid2 = 2;
    const auto id2 = 2;
    const auto price2 = 100;
    const auto symbol2 = "MSFT";
    const auto action2 = OrderAction::Limit;
    const auto side2 = OrderSide::Bid;
    const auto type2 = OrderType::GoodTillCancel;
    const auto quantity2 = 100;
    Message::Command::PlaceOrder command2 {uid2, id2, symbol2, price2, action2, side2, type2, quantity2};
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {symbol1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new order.
    book.placeOrder(command1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(command1.order_id));
    // Get the first order and check that it is unmodified.
    const auto& order1 = book.getOrder(command1.order_id);
    ASSERT_EQ(order1.status, OrderStatus::Accepted);
    ASSERT_EQ(order1.quantity, quantity1);
    ASSERT_EQ(order1.quantity_to_fill, order1.quantity);
    // Place a second order at the same price level and same quantity as the first order.
    // Both orders should be completely filled.
    book.placeOrder(command2);
    result_receiver.stop();
    t1.join();
    // Second order should have never been inserted into the book.
    ASSERT_FALSE(book.hasOrder(command2.order_id));
    // First order should have been removed.
    ASSERT_FALSE(book.hasOrder(command1.order_id));
    // Check the receiver for result messages.
    auto event1 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(event1.order_id, id2);
    ASSERT_EQ(event1.user_id, uid2);
    ASSERT_EQ(event1.order_price, price2);
    ASSERT_EQ(event1.order_quantity, quantity2);
    auto event2 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(event2.order_id, id1);
    ASSERT_EQ(event2.user_id, uid1);
    ASSERT_EQ(event2.order_price, price1);
    ASSERT_EQ(event2.order_quantity, quantity1);
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

TEST(VectorOrderBookTest, BookShouldMatchOrders2) {
    // Create commands to place orders on different sides of the book, with matchable price levels.
    const auto uid1 = 1;
    const auto id1 = 1;
    const auto price1 = 100;
    const auto symbol1 = "MSFT";
    const auto action1 = OrderAction::Limit;
    const auto side1 = OrderSide::Ask;
    const auto type1 = OrderType::GoodTillCancel;
    const auto quantity1 = 90;
    Message::Command::PlaceOrder command1 {uid1, id1, symbol1, price1, action1, side1, type1, quantity1};
    const auto uid2 = 2;
    const auto id2 = 2;
    const auto price2 = 100;
    const auto symbol2 = "MSFT";
    const auto action2 = OrderAction::Limit;
    const auto side2 = OrderSide::Bid;
    const auto type2 = OrderType::GoodTillCancel;
    const auto quantity2 = 100;
    Message::Command::PlaceOrder command2 {uid2, id2, symbol2, price2, action2, side2, type2, quantity2};
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {symbol1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new order.
    book.placeOrder(command1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(command1.order_id));
    // Get the first order and check that it is unmodified.
    const auto& order1 = book.getOrder(command1.order_id);
    ASSERT_EQ(order1.status, OrderStatus::Accepted);
    ASSERT_EQ(order1.quantity_to_fill, order1.quantity);
    // Place a second order at the same price level but greater quantity than the first order.
    // First order placed should be filled, second order placed should be partially filled.
    book.placeOrder(command2);
    result_receiver.stop();
    t1.join();
    // Second order should have never been inserted into the book.
    ASSERT_TRUE(book.hasOrder(command2.order_id));
    // Get the second order and check that it was matched.
    const auto& order2 = book.getOrder(command2.order_id);
    ASSERT_EQ(order2.status, OrderStatus::PartiallyFilled);
    ASSERT_EQ(order2.quantity_to_fill, quantity2 - quantity1);
    // First order should have been removed.
    ASSERT_FALSE(book.hasOrder(command1.order_id));
    // Check the receiver for result messages.
    auto event1 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(event1.order_id, id1);
    ASSERT_EQ(event1.user_id, uid1);
    ASSERT_EQ(event1.order_quantity, quantity1);
    ASSERT_EQ(event1.order_price, price1);
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

TEST(VectorOrderBookTest, BookShouldMatchOrders3) {
    // Create commands to place orders on different sides of the book, with multiple matchable price levels.
    const auto uid1 = 1;
    const auto id1 = 1;
    const auto price1 = 100;
    const auto symbol1 = "MSFT";
    const auto action1 = OrderAction::Limit;
    const auto side1 = OrderSide::Ask;
    const auto type1 = OrderType::GoodTillCancel;
    const auto quantity1 = 90;
    Message::Command::PlaceOrder command1 {uid1, id1, symbol1, price1, action1, side1, type1, quantity1};
    const auto uid2 = 2;
    const auto id2= 2;
    const auto price2 = 120;
    const auto symbol2 = "MSFT";
    const auto action2 = OrderAction::Limit;
    const auto side2 = OrderSide::Ask;
    const auto type2 = OrderType::GoodTillCancel;
    const auto quantity2 = 200;
    Message::Command::PlaceOrder command2 {uid2, id2, symbol2, price2, action2, side2, type2, quantity2};
    const auto uid3 = 3;
    const auto id3 = 3;
    const auto price3 = 200;
    const auto symbol3 = "MSFT";
    const auto action3 = OrderAction::Limit;
    const auto side3 = OrderSide::Bid;
    const auto type3 = OrderType::GoodTillCancel;
    const auto quantity3 = 200;
    Message::Command::PlaceOrder command3 {uid3, id3, symbol3, price3, action3, side3, type3, quantity3};
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {symbol1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new order.
    book.placeOrder(command1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(command1.order_id));
    // Get the first order and check that it is unmodified.
    const auto& order1 = book.getOrder(command1.order_id);
    ASSERT_EQ(order1.status, OrderStatus::Accepted);
    ASSERT_EQ(order1.quantity_to_fill, order1.quantity);
    // Place new order.
    book.placeOrder(command2);
    // Book should now contain the second order since it has no match.
    ASSERT_TRUE(book.hasOrder(command2.order_id));
    // Get the second order and check that it is unmodified.
    const auto& order2 = book.getOrder(command1.order_id);
    ASSERT_EQ(order2.status, OrderStatus::Accepted);
    ASSERT_EQ(order2.quantity_to_fill, order2.quantity);
    // Place new order. This order should match with the previous two orders.
    book.placeOrder(command3);
    ASSERT_FALSE(book.hasOrder(command3.order_id));
    result_receiver.stop();
    t1.join();
    // Second order should still be in the book since it was not fully filled.
    ASSERT_TRUE(book.hasOrder(command2.order_id));
    const auto& order2_matched = book.getOrder(command2.order_id);
    ASSERT_EQ(order2_matched.status, OrderStatus::PartiallyFilled);
    ASSERT_EQ(order2_matched.quantity_to_fill, quantity2 - (quantity3 - quantity1));
    // First order should have been removed since it was filled.
    ASSERT_FALSE(book.hasOrder(command1.order_id));
    // Check the receiver for result messages.
    auto event1 = result_receiver.trade_events.back();
    result_receiver.trade_events.pop_back();
    ASSERT_EQ(event1.order_id, id2);
    ASSERT_EQ(event1.user_id, uid2);
    ASSERT_EQ(event1.matched_order_id, id3);
    ASSERT_EQ(event1.quantity, quantity3 - quantity1);
    ASSERT_EQ(event1.order_price, price2);
    ASSERT_EQ(event1.matched_order_price, price3);
    auto event2 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(event2.order_id, id3);
    ASSERT_EQ(event2.user_id, uid3);
    ASSERT_EQ(event2.order_quantity, quantity3);
    ASSERT_EQ(event2.order_price, price3);
    auto event3 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(event3.order_id, id1);
    ASSERT_EQ(event3.user_id, uid1);
    ASSERT_EQ(event3.order_quantity, quantity1);
    ASSERT_EQ(event3.order_price, price1);
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}
