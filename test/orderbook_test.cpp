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
 * Order book should handle placing a GTC order when the book is empty.
 * Order should be unmatched since there are no other orders to match with and
 * should be inserted into the order book.
 */
TEST(VectorOrderBookTest, BookShouldHandleUnmatchedOrders1) {
    // Create a GTC order.
    Order order = Order::askLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place a new order.
    book.placeOrder(order);
    // Close out the message queue.
    result_receiver.stop();
    t1.join();
    // Book should now contain the order since it is a GTC order and has no match.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    // No events have occurred because the order placed was GTC and because it was unmatched - there should
    // not be any messages.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
    ASSERT_TRUE(result_receiver.orders_rejected.empty());
}

/**
 * Order book should handle placing a IOC order when the book is empty.
 * Order should be unmatched since there are no other orders to match with and
 * should then be cancelled.
 */
TEST(VectorOrderBookTest, BookShouldHandleUnmatchedOrders2) {
    // Create an IOC order.
    Order order = Order::askLimit(OrderType::ImmediateOrCancel, 200, 100, 1, 1, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place the IOC order.
    book.placeOrder(order);
    // Close out the message queue.
    result_receiver.stop();
    t1.join();
    // Book should not contain the order since it is an IOC order - IOC orders are never inserted into the book.
    ASSERT_FALSE(book.hasOrder(1));
    // Order should have been rejected since it was unmatched.
    auto rejection_event1 = result_receiver.orders_rejected.back();
    ASSERT_EQ(1, rejection_event1.order_id);
    ASSERT_EQ(1, rejection_event1.user_id);
    ASSERT_EQ(200, rejection_event1.quantity_rejected);
    ASSERT_EQ(1, rejection_event1.symbol_id);
    result_receiver.orders_rejected.pop_back();
    // There should not be any other messages other than the single rejection.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
    ASSERT_TRUE(result_receiver.orders_rejected.empty());
}

/**
 * Order book should handle placing GTC orders when the book is not empty.
 * Orders should be unmatched since they are on the same side of the book.
 */
TEST(VectorOrderBookTest, BookShouldHandleUnmatchedOrders3) {
    // Create GTC orders on same side of book.
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
    book.placeOrder(order2);
    // Close out the message queue.
    result_receiver.stop();
    t1.join();
    // Book should now contain both orders since they are GTC orders that have no match.
    // Orders on the same side of the book should never match with one another.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    ASSERT_EQ(book.getOrder(2).quantity_executed, 0);
    // Neither order should have been matched nor rejected - there should not be any messages.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
    ASSERT_TRUE(result_receiver.orders_rejected.empty());
}

/**
 * Order book should handle placing GTC orders on the same side of the book.
 * Orders should not be matched since they are not at matchable prices.
 */
TEST(VectorOrderBookTest, BookShouldHandleUnmatchedOrders4) {
    // Create GTC orders on different sides of the book, but unmatchable price levels.
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
    book.placeOrder(order2);
    // Close out the message queue.
    result_receiver.stop();
    t1.join();
    // Book should now contain both orders since they are GTC orders that have no match.
    // Orders on the same side of the book should never match with one another.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    ASSERT_EQ(book.getOrder(2).quantity_executed, 0);
    // Neither order should have been rejected nor traded - there should not be any messages.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
    ASSERT_TRUE(result_receiver.orders_rejected.empty());
}

/**
 * Order book should handle placing a FOK order that cannot be filled in its entirety.
 * The order should be rejected and none of the GTC orders should have been modified.
 */
TEST(VectorOrderBookTest, BookShouldHandleUnmatchedOrders5) {
    // Create GTC orders.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 40, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 120, 50, 2, 2, 1);
    Order order3 = Order::askLimit(OrderType::GoodTillCancel, 200, 110, 3, 3, 1);
    // Create FOK order that cannot be filled in its entirety.
    Order order4 = Order::bidLimit(OrderType::FillOrKill, 175, 105, 4, 4, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new GTC orders.
    book.placeOrder(order1);
    book.placeOrder(order2);
    book.placeOrder(order3);
    // Place FOK order.
    book.placeOrder(order4);
    // Close out the message queue.
    result_receiver.stop();
    t1.join();
    // Book should contain all GTC orders since the FOK order could not be executed in full.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_TRUE(book.hasOrder(3));
    // None of the GTC order should have been traded.
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    ASSERT_EQ(book.getOrder(2).quantity_executed, 0);
    ASSERT_EQ(book.getOrder(3).quantity_executed, 0);
    // FOK should have been rejected since it could not be executed in full.
    auto rejection_event1 = result_receiver.orders_rejected.back();
    result_receiver.orders_rejected.pop_back();
    ASSERT_EQ(4, rejection_event1.order_id);
    ASSERT_EQ(4, rejection_event1.user_id);
    ASSERT_EQ(175, rejection_event1.quantity_rejected);
    ASSERT_EQ(1, rejection_event1.symbol_id);
    // There should not be any other messages.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
    ASSERT_TRUE(result_receiver.orders_rejected.empty());
}

/**
 * Order book should handle cancelling an order when it is the only order in the book.
 */
TEST(VectorOrderBookTest, BookShouldCancelOrders1) {
    // Create a GTC order.
    Order order = Order::askLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place a new GTC order.
    book.placeOrder(order);
    // Order should be inserted into the book since there are no other orders to match with.
    ASSERT_TRUE(book.hasOrder(1));
    // Cancel the order that was just placed.
    book.cancelOrder(1);
    // The order was cancelled - it should no longer be in the book.
    ASSERT_FALSE(book.hasOrder(1));
    // Close out the message queue.
    result_receiver.stop();
    t1.join();
    auto rejection_event1 = result_receiver.orders_rejected.back();
    result_receiver.orders_rejected.pop_back();
    ASSERT_EQ(1, rejection_event1.order_id);
    ASSERT_EQ(1, rejection_event1.user_id);
    ASSERT_EQ(200, rejection_event1.quantity_rejected);
    ASSERT_EQ(1, rejection_event1.symbol_id);
    // Order should not have been traded, so no other messages should have been sent.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
    ASSERT_TRUE(result_receiver.orders_rejected.empty());
}

/**
 * Order book should handle cancelling an order when there are multiple orders in the book.
 */
TEST(VectorOrderBookTest, BookShouldCancelOrders2) {
    // Create GTC orders on the same side of the book.
    Order order1 = Order::bidLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    Order order2 = Order::bidLimit(OrderType::GoodTillCancel, 200, 100, 2, 2, 1);
    Order order3 = Order::bidLimit(OrderType::GoodTillCancel, 200, 100, 3, 3, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place the GTC orders - all orders should be inserted since there are no orders to match with.
    book.placeOrder(order1);
    ASSERT_TRUE(book.hasOrder(1));
    book.placeOrder(order2);
    ASSERT_TRUE(book.hasOrder(2));
    book.placeOrder(order3);
    ASSERT_TRUE(book.hasOrder(3));
    // Cancel the second order the was placed.
    book.cancelOrder(2);
    // The second order was cancelled and so it should no longer be in the book.
    ASSERT_FALSE(book.hasOrder(2));
    // Close out the message queue.
    result_receiver.stop();
    t1.join();
    auto rejection_event1 = result_receiver.orders_rejected.back();
    result_receiver.orders_rejected.pop_back();
    ASSERT_EQ(2, rejection_event1.order_id);
    ASSERT_EQ(2, rejection_event1.user_id);
    ASSERT_EQ(200, rejection_event1.quantity_rejected);
    ASSERT_EQ(1, rejection_event1.symbol_id);
    // Order should not have been traded, so no other messages should have been sent.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
    ASSERT_TRUE(result_receiver.orders_rejected.empty());
}

/**
 * Order book should handle a single match between GTC orders where both orders are fully executed.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders1) {
    // Create GTC orders on different sides of the book, with matchable price levels.
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
    // Place a second order at the same price level and same quantity as the first order.
    // Both orders should be completely filled.
    book.placeOrder(order2);
    result_receiver.stop();
    t1.join();
    // Second order should have never been inserted into the book.
    ASSERT_FALSE(book.hasOrder(2));
    // First order should have been removed.
    ASSERT_FALSE(book.hasOrder(1));
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
    ASSERT_TRUE(result_receiver.orders_rejected.empty());
}

/**
 * Order book should be able to handle a single match between GTC orders where one order is fully executed and
 * the other other is only partially executed.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders2) {
    // Create GTC orders on different sides of the book, with matchable price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 90, 100, 1, 1, 1);
    Order order2 = Order::bidLimit(OrderType::GoodTillCancel, 100, 100, 2, 2, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new GTC order.
    book.placeOrder(order1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(1));
    // Get the first order and check that it is unmodified.
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    // Place a second GTC order at the same price level but greater quantity than the first order.
    // First order placed should be filled, second order placed should be partially filled.
    book.placeOrder(order2);
    // Close out the message queue.
    result_receiver.stop();
    t1.join();
    // Second order should have been inserted into the book since it was unfilled.
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_EQ(book.getOrder(2).quantity_executed, 90);
    // First order should have been removed from the book.
    ASSERT_FALSE(book.hasOrder(1));
    // There should be a message indicating that the first order was executed.
    auto execution_event1 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(execution_event1.order_id, 1);
    ASSERT_EQ(execution_event1.user_id, 1);
    ASSERT_EQ(execution_event1.order_quantity, 90);
    ASSERT_EQ(execution_event1.order_price, 100);
    // There should be a message indicating that the second order was traded with the first
    auto trade_event1 = result_receiver.trade_events.back();
    result_receiver.trade_events.pop_back();
    ASSERT_EQ(trade_event1.order_id, 2);
    ASSERT_EQ(trade_event1.user_id, 2);
    ASSERT_EQ(trade_event1.matched_order_id, 1);
    ASSERT_EQ(trade_event1.matched_order_price, 100);
    ASSERT_EQ(trade_event1.quantity, 90);
    // There should not be any other messages. 
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
    ASSERT_TRUE(result_receiver.orders_rejected.empty());
}

/**
 * Order book should be able to handle multiple matches between GTC orders - a GTC order is placed that matches with
 * multiple orders at different price levels.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders3) {
    // Create GTC orders on different sides of the book with multiple matchable price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 90, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 200, 120, 2, 2, 1);
    Order order3 = Order::bidLimit(OrderType::GoodTillCancel, 200, 200, 3, 3, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place new GTC order.
    book.placeOrder(order1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    // Place a new GTC order.
    book.placeOrder(order2);
    // Book should now contain the second order since it has no match.
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_EQ(book.getOrder(2).quantity_executed, 0);
    // Place new GTC order on the opposite side of the book as the previous two orders. 
    // This order should match with the previous two orders.
    book.placeOrder(order3);
    // Order should have been executed and therefore never inserted into the book.
    ASSERT_FALSE(book.hasOrder(3));
    // Second order should still be in the book since it was not fully filled.
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_EQ(book.getOrder(2).quantity_executed, 200 - 90);
    // First order should have been removed since it was fully filled.
    ASSERT_FALSE(book.hasOrder(1));
    // Close out the message queue.
    result_receiver.stop();
    t1.join();
    // There should be a message indicating that the second order was traded with the third order.
    auto trade_event1 = result_receiver.trade_events.back();
    result_receiver.trade_events.pop_back();
    ASSERT_EQ(trade_event1.order_id, 2);
    ASSERT_EQ(trade_event1.user_id, 2);
    ASSERT_EQ(trade_event1.matched_order_id, 3);
    ASSERT_EQ(trade_event1.quantity, 200 - 90);
    ASSERT_EQ(trade_event1.order_price, 120);
    ASSERT_EQ(trade_event1.matched_order_price, 200);
    // There should be a message indicating that the third order was traded with the first order.
    auto trade_event2 = result_receiver.trade_events.back();
    result_receiver.trade_events.pop_back();
    ASSERT_EQ(trade_event2.order_id, 3);
    ASSERT_EQ(trade_event2.user_id, 3);
    ASSERT_EQ(trade_event2.matched_order_id, 1);
    ASSERT_EQ(trade_event2.quantity, 90);
    ASSERT_EQ(trade_event2.order_price, 200);
    ASSERT_EQ(trade_event2.matched_order_price, 100);
    // There should be a message indicating that the third order was executed.
    auto execution_event1 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(execution_event1.order_id, 3);
    ASSERT_EQ(execution_event1.user_id, 3);
    ASSERT_EQ(execution_event1.order_quantity, 200);
    ASSERT_EQ(execution_event1.order_price, 200);
    // There should be a message indicating that the first order was executed.
    auto execution_event3 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(execution_event3.order_id, 1);
    ASSERT_EQ(execution_event3.user_id, 1);
    ASSERT_EQ(execution_event3.order_quantity, 90);
    ASSERT_EQ(execution_event3.order_price, 100);
    // There should be no other messages.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
    ASSERT_TRUE(result_receiver.orders_rejected.empty());
}

/**
 * Order book should be able to handle an IOC order that is matches with GTC orders at multiple
 * price levels and is fully filled.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders4) {
    // Create GTC orders on same side of the book with different price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 90, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 100, 110, 2, 2, 1);
    // Create IOC order on opposite side of book as GTC orders.
    Order order3 = Order::bidLimit(OrderType::ImmediateOrCancel, 190, 150, 3, 3, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place the first GTC order.
    book.placeOrder(order1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(1));
    // Get the first order and check that it is unmodified.
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    // Place a second GTC order.
    book.placeOrder(order2);
    // Book should now contain the second order since it has no match.
    ASSERT_TRUE(book.hasOrder(1));
    // Get the second order and check that it is unmodified.
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    // Place the FOK order - should match with the previous GTC orders.
    book.placeOrder(order3);
    // Book should not contain the IOC order - IOC orders are never inserted into the book.
    ASSERT_FALSE(book.hasOrder(3));
    // GTC orders should have been matched and removed from the book.
    ASSERT_FALSE(book.hasOrder(2));
    ASSERT_FALSE(book.hasOrder(1));
    // Close out the queue.
    result_receiver.stop();
    t1.join();
    // There should be a message indicating that the third order was executed.
    auto execution_event3 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(execution_event3.order_id, 3);
    ASSERT_EQ(execution_event3.user_id, 3);
    ASSERT_EQ(execution_event3.order_quantity, 190);
    ASSERT_EQ(execution_event3.order_price, 150);
    // There should be a message indicating that the second order was executed.
    auto execution_event2 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(execution_event2.order_id, 2);
    ASSERT_EQ(execution_event2.user_id, 2);
    ASSERT_EQ(execution_event2.order_quantity, 100);
    ASSERT_EQ(execution_event2.order_price, 110);
    // There should be a message indicating that the first order was executed.
    auto execution_event1 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(execution_event1.order_id, 1);
    ASSERT_EQ(execution_event1.user_id, 1);
    ASSERT_EQ(execution_event1.order_quantity, 90);
    ASSERT_EQ(execution_event1.order_price, 100);
    // There should be a message indicating that the first order was traded with the third.
    auto trade_event1 = result_receiver.trade_events.back();
    result_receiver.trade_events.pop_back();
    ASSERT_EQ(trade_event1.order_id, 3);
    ASSERT_EQ(trade_event1.user_id, 3);
    ASSERT_EQ(trade_event1.matched_order_id, 1);
    ASSERT_EQ(trade_event1.matched_order_price, 100);
    ASSERT_EQ(trade_event1.quantity, 90);
    // There should not be any other messages.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
    ASSERT_TRUE(result_receiver.orders_rejected.empty());
}

/**
 * Order book should be able to handle an FOK order that is fully executable.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders5) {
    // Create GTC orders on same side of the book with different price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 90, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 100, 110, 2, 2, 1);
    Order order3 = Order::askLimit(OrderType::GoodTillCancel, 50, 110, 3, 3, 1);
    // Create FOK order on opposite side of book as GTC orders.
    Order order4 = Order::bidLimit(OrderType::FillOrKill, 200, 150, 4, 4, 1);
    // Create a messenger for the order book.
    OrderBookReceiver result_receiver;
    // Start up a worker waiting for results.
    std::thread t1 {&OrderBookReceiver::start, &result_receiver};
    // Create the order book.
    OrderBook::VectorOrderBook book {1, static_cast<Messaging::Sender>(result_receiver.receiver)};
    // Place the first GTC order.
    book.placeOrder(order1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(1));
    // Get the first order and check that it is unmodified.
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    // Place a second GTC order.
    book.placeOrder(order2);
    // Book should now contain the second order since it has no match.
    ASSERT_TRUE(book.hasOrder(2));
    // Get the second order and check that it is unmodified.
    ASSERT_EQ(book.getOrder(2).quantity_executed, 0);
    // Place a third GTC order.
    book.placeOrder(order3);
    // Book should now contain the third order since it has no match.
    ASSERT_TRUE(book.hasOrder(3));
    // Get the second order and check that it is unmodified.
    ASSERT_EQ(book.getOrder(3).quantity_executed, 0);
    // Place the FOK order - should match with the previous GTC orders.
    book.placeOrder(order4);
    // Book should not contain the FOK order or the orders that it was executed with.
    ASSERT_FALSE(book.hasOrder(4));
    ASSERT_FALSE(book.hasOrder(2));
    ASSERT_FALSE(book.hasOrder(1));
    // This order should not have been fully filled and should still be in the book.
    ASSERT_TRUE(book.hasOrder(3));
    ASSERT_EQ(book.getOrder(3).quantity_executed, 200 - 90 - 100);
    // Close out the queue.
    result_receiver.stop();
    t1.join();
    // There should be a message indicating that the FOK order was executed.
    auto execution_event3 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(execution_event3.order_id, 4);
    ASSERT_EQ(execution_event3.user_id, 4);
    ASSERT_EQ(execution_event3.order_quantity, 200);
    ASSERT_EQ(execution_event3.order_price, 150);
    // There should be a message indicating that the second GTC order was executed.
    auto execution_event2 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(execution_event2.order_id, 2);
    ASSERT_EQ(execution_event2.user_id, 2);
    ASSERT_EQ(execution_event2.order_quantity, 100);
    ASSERT_EQ(execution_event2.order_price, 110);
    // There should be a message indicating that the first GTC order was executed.
    auto execution_event1 = result_receiver.orders_executed.back();
    result_receiver.orders_executed.pop_back();
    ASSERT_EQ(execution_event1.order_id, 1);
    ASSERT_EQ(execution_event1.user_id, 1);
    ASSERT_EQ(execution_event1.order_quantity, 90);
    ASSERT_EQ(execution_event1.order_price, 100);
    // There should be a message indicating that the FOK order was traded with the third GTC order.
    auto trade_event1 = result_receiver.trade_events.back();
    result_receiver.trade_events.pop_back();
    ASSERT_EQ(trade_event1.order_id, 3);
    ASSERT_EQ(trade_event1.user_id, 3);
    ASSERT_EQ(trade_event1.matched_order_id, 4);
    ASSERT_EQ(trade_event1.matched_order_price, 150);
    ASSERT_EQ(trade_event1.quantity, 200 - 90 - 100);
    // There should be a message indicating that the FOK order was traded with the second GTC order.
    auto trade_event2 = result_receiver.trade_events.back();
    result_receiver.trade_events.pop_back();
    ASSERT_EQ(trade_event2.order_id, 4);
    ASSERT_EQ(trade_event2.user_id, 4);
    ASSERT_EQ(trade_event2.matched_order_id, 2);
    ASSERT_EQ(trade_event2.matched_order_price, 110);
    ASSERT_EQ(trade_event2.quantity, 100);
    // There should be a message indicating that the FOK order was traded with the first GTC order.
    auto trade_event3 = result_receiver.trade_events.back();
    result_receiver.trade_events.pop_back();
    ASSERT_EQ(trade_event3.order_id, 4);
    ASSERT_EQ(trade_event3.user_id, 4);
    ASSERT_EQ(trade_event3.matched_order_id, 1);
    ASSERT_EQ(trade_event3.matched_order_price, 100);
    ASSERT_EQ(trade_event3.quantity, 90);
    // There should not be any other messages.
    ASSERT_TRUE(result_receiver.trade_events.empty());
    ASSERT_TRUE(result_receiver.orders_executed.empty());
    ASSERT_TRUE(result_receiver.orders_rejected.empty());
}