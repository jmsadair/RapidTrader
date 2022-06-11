#include <gtest/gtest.h>
#include <thread>
#include "vector_orderbook.h"

struct OrderBookReceiver {
    Messaging::Receiver receiver;
    std::vector<Message::Event::TradeEvent> trade_events {};
    std::vector<Message::Event::OrderExecuted> orders_executed {};
    std::vector<Message::Event::RejectionEvent> orders_rejected {};
    void start() {
        try {
            while (true) {
                receiver.wait()
                        .handle<Message::Event::TradeEvent>([&](const Message::Event::TradeEvent& msg) {
                            trade_events.push_back(msg);
                        })
                        .handle<Message::Event::OrderExecuted>([&](const Message::Event::OrderExecuted& msg) {
                            orders_executed.push_back(msg);
                        })
                        .handle<Message::Event::RejectionEvent>([&](const Message::Event::RejectionEvent& msg) {
                            orders_rejected.push_back(msg);
                        });
            }
        } catch(const Messaging::CloseQueue&) {}
    }
    void stop() {
        auto sender = static_cast<Messaging::Sender>(receiver);
        sender.send(Messaging::CloseQueue());
    }
};

/**
 * Order book should handle placing an order when the book is empty.
 * Order should be unmatched since there are no other orders to match with.
 */
TEST(VectorOrderBookTest, BookShouldInsertUnmatchedOrders1) {
    // Create a command to place an order.
    Order order = Order::askLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place a new order.
    book.placeOrder(order);
    result_receiver.stop();
    t1.join();
    // Book should now contain the order since it is a GTC order and has no match.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    // Minimum ask price should now be the price of the order.
    ASSERT_EQ(book.minAskPrice(), 100);
    // Order should not have been traded, so no message should have been sent.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

/**
 * Order book should handle placing an order when the book is not empty.
 * Orders should be unmatched since they are on the same side of the book.
 */
TEST(VectorOrderBookTest, BookShouldInsertUnmatchedOrders2) {
    // Create commands to place orders on same side of book.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 120, 200, 2, 2, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new orders.
    book.placeOrder(order1);
    // Minimum ask price should now be the price of the order just placed.
    ASSERT_EQ(book.minAskPrice(), 100);
    book.placeOrder(order2);
    // Minimum ask price should remain unchanged.
    ASSERT_EQ(book.minAskPrice(), 100);
    result_receiver.stop();
    t1.join();
    // Book should now contain both orders since they are GTC orders that have no match.
    // Orders on the same side of the book should never match with one another.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    ASSERT_EQ(book.getOrder(2).quantity_executed, 0);
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

/**
 * Order book should handle placing orders on the opposite side of the book.
 * Orders should not be matched since they are not at matchable prices.
 */
TEST(VectorOrderBookTest, BookShouldInsertUnmatchedOrders3) {
    // Create commands to place orders on different sides of the book, but unmatchable price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 120, 50, 2, 2, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new orders.
    book.placeOrder(order1);
    // Minimum ask price should now be the price of the order just placed.
    ASSERT_EQ(book.minAskPrice(), 100);
    book.placeOrder(order2);
    // Minimum ask price should now be the price of the newly placed order since it lesser.
    ASSERT_EQ(book.minAskPrice(), 50);
    result_receiver.stop();
    t1.join();
    // Book should now contain both orders since they are GTC orders that have no match.
    // Orders on the same side of the book should never match with one another.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    ASSERT_EQ(book.getOrder(2).quantity_executed, 0);
    // Check the receiver for result message.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

/**
 * Order book should handle cancelling an order when it is the only order in the book.
 */
TEST(VectorOrderBookTest, BookShouldCancelOrders1) {
    // Create order.
    Order order = Order::askLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place a new order.
    book.placeOrder(order);
    ASSERT_TRUE(book.hasOrder(1));
    // Minimum ask price should now be the price of the order just placed.
    ASSERT_EQ(book.minAskPrice(), 100);
    book.cancelOrder(1);
    ASSERT_FALSE(book.hasOrder(1));
    result_receiver.stop();
    t1.join();
    auto event1 = result_receiver.orders_rejected.back();
    ASSERT_EQ(1, event1.order_id);
    ASSERT_EQ(1, event1.user_id);
    ASSERT_EQ(200, event1.quantity_rejected);
    ASSERT_EQ(1, event1.symbol_id);
    // Order should not have been traded, so no message should have been sent.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

/**
 * Order book should handle cancelling an order when there are multiple orders in the book.
 */
TEST(VectorOrderBookTest, BookShouldCancelOrders2) {
    // Create orders on the same side of the book.
    Order order1 = Order::bidLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    Order order2 = Order::bidLimit(OrderType::GoodTillCancel, 200, 100, 2, 2, 1);
    Order order3 = Order::bidLimit(OrderType::GoodTillCancel, 200, 100, 3, 3, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place a new order.
    book.placeOrder(order1);
    ASSERT_TRUE(book.hasOrder(1));
    // Maximum bid price should now be the price of the order just placed.
    ASSERT_EQ(book.maxBidPrice(), 100);
    book.placeOrder(order2);
    ASSERT_TRUE(book.hasOrder(2));
    // Maximum bid price should remain unchanged.
    ASSERT_EQ(book.maxBidPrice(), 100);
    book.placeOrder(order3);
    ASSERT_TRUE(book.hasOrder(3));
    // Maximum bid price should remain unchanged.
    ASSERT_EQ(book.maxBidPrice(), 100);
    book.cancelOrder(2);
    ASSERT_FALSE(book.hasOrder(2));
    // Maximum bid price should remain unchanged.
    ASSERT_EQ(book.maxBidPrice(), 100);
    result_receiver.stop();
    t1.join();
    auto event1 = result_receiver.orders_rejected.back();
    ASSERT_EQ(2, event1.order_id);
    ASSERT_EQ(2, event1.user_id);
    ASSERT_EQ(200, event1.quantity_rejected);
    ASSERT_EQ(1, event1.symbol_id);
    // Order should not have been traded, so no message should have been sent.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

/**
 * Order book should handle a single match where both orders are fully executed.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders1) {
    // Create orders on different sides of the book, with matchable price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 100, 100, 1, 1, 1);
    Order order2 = Order::bidLimit(OrderType::GoodTillCancel, 100, 100, 2, 2, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new order.
    book.placeOrder(order1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    // Minimum ask price should now be the price of the order just placed.
    ASSERT_EQ(book.minAskPrice(), 100);
    // Place a second order at the same price level and same quantity as the first order.
    // Both orders should be completely filled.
    book.placeOrder(order2);
    result_receiver.stop();
    t1.join();
    // Second order should have never been inserted into the book.
    ASSERT_FALSE(book.hasOrder(2));
    // First order should have been removed.
    ASSERT_FALSE(book.hasOrder(1));
    // Minimum ask price and maximum bidding price should have been reset since the book is now empty.
    ASSERT_EQ(book.minAskPrice(), std::numeric_limits<uint32_t>::max());
    ASSERT_EQ(book.maxBidPrice(), 0);
    // Check the receiver for result message.
    auto event1 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(event1.order_id, 2);
    ASSERT_EQ(event1.user_id, 2);
    ASSERT_EQ(event1.order_price, 100);
    ASSERT_EQ(event1.order_quantity, 100);
    auto event2 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(event2.order_id, 1);
    ASSERT_EQ(event2.user_id, 1);
    ASSERT_EQ(event2.order_price, 100);
    ASSERT_EQ(event2.order_quantity, 100);
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

/**
 * Order book should be able to handle a single match where one order is fully executed and
 * the other other is only partially executed.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders2) {
    // Create orders on different sides of the book, with matchable price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 90, 100, 1, 1, 1);
    Order order2 = Order::bidLimit(OrderType::GoodTillCancel, 100, 100, 2, 2, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new order.
    book.placeOrder(order1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(1));
    // Minimum ask price should now be the price of the order just placed.
    ASSERT_EQ(book.minAskPrice(), 100);
    // Get the first order and check that it is unmodified.
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    // Place a second order at the same price level but greater quantity than the first order.
    // First order placed should be filled, second order placed should be partially filled.
    book.placeOrder(order2);
    result_receiver.stop();
    t1.join();
    // Second order should have never been inserted into the book.
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_EQ(book.getOrder(2).quantity_executed, 90);
    // First order should have been removed.
    ASSERT_FALSE(book.hasOrder(1));
    // Check the receiver for result message.
    auto event1 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(event1.order_id, 1);
    ASSERT_EQ(event1.user_id, 1);
    ASSERT_EQ(event1.order_quantity, 90);
    ASSERT_EQ(event1.order_price, 100);
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}

/**
 * Order book should be able to handle multiple matches - an order is placed that matches with
 * multiple orders at different price levels.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders3) {
    // Create orders on different sides of the book with multiple matchable price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 90, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 200, 120, 2, 2, 1);
    Order order3 = Order::bidLimit(OrderType::GoodTillCancel, 200, 200, 3, 3, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new order.
    book.placeOrder(order1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    // Place new order.
    book.placeOrder(order2);
    // Book should now contain the second order since it has no match.
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_EQ(book.getOrder(2).quantity_executed, 0);
    // Place new order. This order should match with the previous two orders.
    book.placeOrder(order3);
    ASSERT_FALSE(book.hasOrder(3));
    // Second order should still be in the book since it was not fully filled.
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_EQ(book.getOrder(2).quantity_executed, 200 - 90);
    // First order should have been removed since it was filled.
    ASSERT_FALSE(book.hasOrder(1));
    // Check the receiver for result message.
    result_receiver.stop();
    t1.join();
    auto event1 = result_receiver.trade_events.back();
    result_receiver.trade_events.pop_back();
    ASSERT_EQ(event1.order_id, 2);
    ASSERT_EQ(event1.user_id, 2);
    ASSERT_EQ(event1.matched_order_id, 3);
    ASSERT_EQ(event1.quantity, 200 - 90);
    ASSERT_EQ(event1.order_price, 120);
    ASSERT_EQ(event1.matched_order_price, 200);
    auto event2 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(event2.order_id, 3);
    ASSERT_EQ(event2.user_id, 3);
    ASSERT_EQ(event2.order_quantity, 200);
    ASSERT_EQ(event2.order_price, 200);
    auto event3 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(event3.order_id, 1);
    ASSERT_EQ(event3.user_id, 1);
    ASSERT_EQ(event3.order_quantity, 90);
    ASSERT_EQ(event3.order_price, 100);
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
}
