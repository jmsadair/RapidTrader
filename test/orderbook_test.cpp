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
    const auto symbol_id = 1;
    const auto type = OrderType::GoodTillCancel;
    const auto quantity = 200;
    Order order = Order::askLimit(type, quantity, price, id, uid, symbol_id);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {symbol_id, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place a new order.
    book.placeOrder(order);
    result_receiver.stop();
    t1.join();
    // Book should now contain the order since it is a GTC order and has no match.
    ASSERT_TRUE(book.hasOrder(id));
    ASSERT_EQ(order.quantity_executed, 0);
    // Order should not have been traded, so no message should have been sent.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

TEST(VectorOrderBookTest, BookShouldInsertUnmatchedOrders2) {
    // Create commands to place orders on same side of book.
    const auto uid1 = 1;
    const auto id1 = 1;
    const auto price1 = 100;
    const auto symbol_id1 = 1;
    const auto type1 = OrderType::GoodTillCancel;
    const auto quantity1 = 200;
    Order order1 = Order::askLimit(type1, quantity1, price1, id1, uid1, symbol_id1);
    const auto uid2 = 2;
    const auto id2 = 2;
    const auto price2 = 200;
    const auto symbol_id2 = 1;
    const auto type2 = OrderType::GoodTillCancel;
    const auto quantity2 = 120;
    Order order2 = Order::askLimit(type2, quantity2, price2, id2, uid2, symbol_id2);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {symbol_id1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new orders.
    book.placeOrder(order1);
    book.placeOrder(order2);
    result_receiver.stop();
    t1.join();
    // Book should now contain both orders since they are GTC orders that have no match.
    // Orders on the same side of the book should never match with one another.
    ASSERT_TRUE(book.hasOrder(id1));
    ASSERT_TRUE(book.hasOrder(id2));
    ASSERT_EQ(order1.quantity_executed, 0);
    ASSERT_EQ(order2.quantity_executed, 0);
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

TEST(VectorOrderBookTest, BookShouldInsertUnmatchedOrders3) {
    // Create commands to place orders on different sides of the book, but unmatchable price levels.
    const auto uid1 = 1;
    const auto id1 = 1;
    const auto price1 = 100;
    const auto symbol_id1 = 1;
    const auto type1 = OrderType::GoodTillCancel;
    const auto quantity1 = 200;
    Order order1 = Order::askLimit(type1, quantity1, price1, id1, uid1, symbol_id1);
    const auto uid2 = 2;
    const auto id2 = 2;
    const auto price2 = 50;
    const auto symbol_id2 = 1;
    const auto type2 = OrderType::GoodTillCancel;
    const auto quantity2 = 120;
    Order order2 = Order::askLimit(type2, quantity2, price2, id2, uid2, symbol_id2);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {symbol_id1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new orders.
    book.placeOrder(order1);
    book.placeOrder(order2);
    result_receiver.stop();
    t1.join();
    // Book should now contain both orders since they are GTC orders that have no match.
    // Orders on the same side of the book should never match with one another.
    ASSERT_TRUE(book.hasOrder(id1));
    ASSERT_TRUE(book.hasOrder(id2));
    ASSERT_EQ(order1.quantity_executed, 0);
    ASSERT_EQ(order2.quantity_executed, 0);
    // Check the receiver for result message.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

TEST(VectorOrderBookTest, BookShouldMatchOrders1) {
    // Create commands to place orders on different sides of the book, with matchable price levels.
    const auto uid1 = 1;
    const auto id1 = 1;
    const auto price1 = 100;
    const auto symbol_id1 = 1;
    const auto type1 = OrderType::GoodTillCancel;
    const auto quantity1 = 100;
    Order order1 = Order::askLimit(type1, quantity1, price1, id1, uid1, symbol_id1);
    const auto uid2 = 2;
    const auto id2 = 2;
    const auto price2 = 100;
    const auto symbol_id2 = 1;
    const auto type2 = OrderType::GoodTillCancel;
    const auto quantity2 = 100;
    Order order2 = Order::bidLimit(type2, quantity2, price2, id2, uid2, symbol_id2);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {symbol_id1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new order.
    book.placeOrder(order1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(id1));
    ASSERT_EQ(order1.quantity_executed, 0);
    // Place a second order at the same price level and same quantity as the first order.
    // Both orders should be completely filled.
    book.placeOrder(order2);
    result_receiver.stop();
    t1.join();
    // Second order should have never been inserted into the book.
    ASSERT_FALSE(book.hasOrder(id2));
    // First order should have been removed.
    ASSERT_FALSE(book.hasOrder(id1));
    // Check the receiver for result message.
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
    const auto symbol_id1 = 1;
    const auto type1 = OrderType::GoodTillCancel;
    const auto quantity1 = 90;
    Order order1 = Order::askLimit(type1, quantity1, price1, id1, uid1, symbol_id1);
    const auto uid2 = 2;
    const auto id2 = 2;
    const auto price2 = 100;
    const auto symbol_id2 = 1;
    const auto type2 = OrderType::GoodTillCancel;
    const auto quantity2 = 100;
    Order order2 = Order::bidLimit(type2, quantity2, price2, id2, uid2, symbol_id2);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {symbol_id1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new order.
    book.placeOrder(order1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(id1));
    // Get the first order and check that it is unmodified.
    const auto& inserted_order1 = book.getOrder(id1);
    ASSERT_EQ(inserted_order1.quantity_executed, 0);
    // Place a second order at the same price level but greater quantity than the first order.
    // First order placed should be filled, second order placed should be partially filled.
    book.placeOrder(order2);
    result_receiver.stop();
    t1.join();
    // Second order should have never been inserted into the book.
    ASSERT_TRUE(book.hasOrder(id2));
    ASSERT_EQ(order2.quantity_executed, quantity1);
    // First order should have been removed.
    ASSERT_FALSE(book.hasOrder(id1));
    // Check the receiver for result message.
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
    const auto symbol_id1 = 1;
    const auto type1 = OrderType::GoodTillCancel;
    const auto quantity1 = 90;
    Order order1 = Order::askLimit(type1, quantity1, price1, id1, uid1, symbol_id1);
    const auto uid2 = 2;
    const auto id2= 2;
    const auto price2 = 120;
    const auto symbol_id2 = 1;
    const auto type2 = OrderType::GoodTillCancel;
    const auto quantity2 = 200;
    Order order2 = Order::askLimit(type2, quantity2, price2, id2, uid2, symbol_id2);
    const auto uid3 = 3;
    const auto id3 = 3;
    const auto price3 = 200;
    const auto symbol_id3 = 1;
    const auto type3 = OrderType::GoodTillCancel;
    const auto quantity3 = 200;
    Order order3 = Order::bidLimit(type3, quantity3, price3, id3, uid3, symbol_id3);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {symbol_id1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new order.
    book.placeOrder(order1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(id1));
    ASSERT_EQ(order1.quantity_executed, 0);
    // Place new order.
    book.placeOrder(order2);
    // Book should now contain the second order since it has no match.
    ASSERT_TRUE(book.hasOrder(id2));
    ASSERT_EQ(order2.quantity_executed, 0);
    // Place new order. This order should match with the previous two orders.
    book.placeOrder(order3);
    ASSERT_FALSE(book.hasOrder(id3));
    const Order& order2_ref = book.getOrder(id2);
    // Second order should still be in the book since it was not fully filled.
    ASSERT_TRUE(book.hasOrder(id2));
    ASSERT_EQ(order2.quantity_executed, quantity3 - quantity1);
    // First order should have been removed since it was filled.
    ASSERT_FALSE(book.hasOrder(id1));
    // Check the receiver for result message.
    result_receiver.stop();
    t1.join();
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
