#include <gtest/gtest.h>
#include <thread>
#include "orderbook/vector_orderbook.h"

class DebugEventHandler : public EventHandler
{
public:

    void handleTradeEvent(const Event::TradeEvent &trade_event) override
    {
        trade_events.push_back(trade_event);
    };

    void handleRejectEvent(const Event::RejectionEvent &reject_event) override
    {
        rejection_events.push_back(reject_event);
    };

    void handleExecutionEvent(const Event::OrderExecuted &execution_event) override
    {
        execution_events.push_back(execution_event);
    };

    std::vector<Event::OrderExecuted> execution_events;
    std::vector<Event::RejectionEvent> rejection_events;
    std::vector<Event::TradeEvent> trade_events;
};

/**
 * Order book should handle placing a GTC limit order when the book is empty.
 * Order should be unmatched since there are no other orders to match with and
 * should be inserted into the order book.
 */
TEST(VectorOrderBookTest, BookShouldHandleUnmatchedOrders1)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create a GTC order.
    Order order = Order::askLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    // Place a new order.
    book.placeOrder(order);
    // Book should now contain the order since it is a GTC order and has no match.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    // No events have occurred because the order placed was GTC and because it was unmatched.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should handle placing a IOC limit order when the book is empty.
 * Order should be unmatched since there are no other orders to match with and
 * should then be cancelled.
 */
TEST(VectorOrderBookTest, BookShouldHandleUnmatchedOrders2)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create an IOC order.
    Order order = Order::askLimit(OrderType::ImmediateOrCancel, 200, 100, 1, 1, 1);
    // Place the IOC order.
    book.placeOrder(order);
    // Book should not contain the order since it is an IOC order - IOC orders are never inserted into the book.
    ASSERT_FALSE(book.hasOrder(1));
    // Order should have been rejected since it was unmatched.
    auto rejection_event1 = handler.rejection_events.back();
    ASSERT_EQ(1, rejection_event1.order_id);
    ASSERT_EQ(1, rejection_event1.user_id);
    ASSERT_EQ(200, rejection_event1.quantity_rejected);
    ASSERT_EQ(1, rejection_event1.symbol_id);
    handler.rejection_events.pop_back();
    // There should not be any other messages other than the single rejection.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should handle placing GTC limit orders when the book is not empty.
 * Orders should be unmatched since they are on the same side of the book.
 */
TEST(VectorOrderBookTest, BookShouldHandleUnmatchedOrders3)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create GTC orders on same side of book.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 120, 200, 2, 2, 1);
    // Place new orders.
    book.placeOrder(order1);
    book.placeOrder(order2);
    // Book should now contain both orders since they are GTC orders that have no match.
    // Orders on the same side of the book should never match with one another.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    ASSERT_EQ(book.getOrder(2).quantity_executed, 0);
    // Neither order should have been matched nor rejected - there should not be any messages.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should handle placing GTC limit orders on the same side of the book.
 * Orders should not be matched since they are not at matchable prices.
 */
TEST(VectorOrderBookTest, BookShouldHandleUnmatchedOrders4)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create GTC orders on different sides of the book, but unmatchable price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 120, 50, 2, 2, 1);
    // Place new orders.
    book.placeOrder(order1);
    book.placeOrder(order2);
    // Book should now contain both orders since they are GTC orders that have no match.
    // Orders on the same side of the book should never match with one another.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    ASSERT_EQ(book.getOrder(2).quantity_executed, 0);
    // Neither order should have been rejected nor traded - there should not be any messages.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should handle placing a FOK limit order that cannot be filled in its entirety.
 * The order should be rejected and none of the GTC orders should have been modified.
 */
TEST(VectorOrderBookTest, BookShouldHandleUnmatchedOrders5)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create GTC orders.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 40, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 120, 50, 2, 2, 1);
    Order order3 = Order::askLimit(OrderType::GoodTillCancel, 200, 110, 3, 3, 1);
    // Create FOK order that cannot be filled in its entirety.
    Order order4 = Order::bidLimit(OrderType::FillOrKill, 175, 105, 4, 4, 1);
    // Place new GTC orders.
    book.placeOrder(order1);
    book.placeOrder(order2);
    book.placeOrder(order3);
    // Place FOK order.
    book.placeOrder(order4);
    // Book should contain all GTC orders since the FOK order could not be executed in full.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_TRUE(book.hasOrder(3));
    // None of the GTC order should have been traded.
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    ASSERT_EQ(book.getOrder(2).quantity_executed, 0);
    ASSERT_EQ(book.getOrder(3).quantity_executed, 0);
    // FOK should have been rejected since it could not be executed in full.
    auto rejection_event1 = handler.rejection_events.back();
    handler.rejection_events.pop_back();
    ASSERT_EQ(4, rejection_event1.order_id);
    ASSERT_EQ(4, rejection_event1.user_id);
    ASSERT_EQ(175, rejection_event1.quantity_rejected);
    ASSERT_EQ(1, rejection_event1.symbol_id);
    // There should not be any other messages.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should handle placing a market order that cannot be filled (not enough volume for the
 * symbol). The entire quantity of the order should be cancelled.
 */
TEST(VectorOrderBookTest, BookShouldHandleUnmatchedOrders6)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create a market order.
    Order order1 = Order::askMarket(100, 1, 1, 1);
    // Place the market order.
    book.placeOrder(order1);
    // Book should not contain order since it was cancelled.
    ASSERT_FALSE(book.hasOrder(1));
    // Entire Quantity of the order should have been rejected since the book was empty.
    auto rejection_event1 = handler.rejection_events.back();
    handler.rejection_events.pop_back();
    ASSERT_EQ(1, rejection_event1.order_id);
    ASSERT_EQ(1, rejection_event1.user_id);
    ASSERT_EQ(100, rejection_event1.quantity_rejected);
    ASSERT_EQ(1, rejection_event1.symbol_id);
    // There should not be any other messages.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should handle reducing limit orders.
 */
TEST(VectorOrderBookTest, BookShouldReduceOrders1)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create a GTC order.
    Order order = Order::askLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    // Place a new GTC order.
    book.placeOrder(order);
    // Order should be inserted into the book since there are no other orders to match with.
    ASSERT_TRUE(book.hasOrder(1));
    // Cancel the order that was just placed.
    book.reduceOrder(1, 50);
    // The order was reduced by 50 but should still be in the book.
    ASSERT_TRUE(book.hasOrder(1));
    // Order should now have quantity of 150.
    ASSERT_EQ(book.getOrder(1).quantity, 150);
    // Order should not have been traded, so no other messages should have been sent.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should handle reducing a limit order that is partially executed.
 * The order's quantity is reduced by an amount that places the quantity below the
 * executed quantity, and so the order should be removed from the book.
 */
TEST(VectorOrderBookTest, BookShouldReduceOrders2)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create GTC orders that can match.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    Order order2 = Order::bidLimit(OrderType::GoodTillCancel, 100, 250, 2, 2, 1);
    // Place a new GTC order.
    book.placeOrder(order1);
    // Order should be inserted into the book since there are no other orders to match with.
    ASSERT_TRUE(book.hasOrder(1));
    // Place a new GTC order that can match with the first order placed.
    book.placeOrder(order2);
    // Order should be filled in full and therefore not inserted into the book.
    ASSERT_FALSE(book.hasOrder(2));
    // Reduce the first order by 150. The order has an original quantity of 200 and 100 of those were executed.
    // Because reducing the order by 150 would mean its new quantity is 50, it is removed from the book.
    book.reduceOrder(1, 150);
    ASSERT_FALSE(book.hasOrder(1));
    // Trade event for first order matching with second.
    auto trade_event1 = handler.trade_events.back();
    handler.trade_events.pop_back();
    ASSERT_EQ(trade_event1.order_id, 1);
    ASSERT_EQ(trade_event1.user_id, 1);
    ASSERT_EQ(trade_event1.matched_order_id, 2);
    ASSERT_EQ(trade_event1.matched_order_price, 250);
    ASSERT_EQ(trade_event1.quantity, 100);
    // First order was executed completely due to reduction.
    auto execution_event1 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event1.order_id, 1);
    ASSERT_EQ(execution_event1.user_id, 1);
    ASSERT_EQ(execution_event1.order_price, 100);
    ASSERT_EQ(execution_event1.order_quantity, 100);
    // Second order was executed completely.
    auto execution_event2 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event2.order_id, 2);
    ASSERT_EQ(execution_event2.user_id, 2);
    ASSERT_EQ(execution_event2.order_price, 250);
    ASSERT_EQ(execution_event2.order_quantity, 100);
    // Order should not have been traded, so no other messages should have been sent.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should handle cancelling a limit order when it is the only order in the book.
 */
TEST(VectorOrderBookTest, BookShouldCancelOrders1)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create a GTC order.
    Order order = Order::askLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    // Place a new GTC order.
    book.placeOrder(order);
    // Order should be inserted into the book since there are no other orders to match with.
    ASSERT_TRUE(book.hasOrder(1));
    // Cancel the order that was just placed.
    book.cancelOrder(1);
    // The order was cancelled - it should no longer be in the book.
    ASSERT_FALSE(book.hasOrder(1));
    auto rejection_event1 = handler.rejection_events.back();
    handler.rejection_events.pop_back();
    ASSERT_EQ(1, rejection_event1.order_id);
    ASSERT_EQ(1, rejection_event1.user_id);
    ASSERT_EQ(200, rejection_event1.quantity_rejected);
    ASSERT_EQ(1, rejection_event1.symbol_id);
    // Order should not have been traded, so no other messages should have been sent.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should handle cancelling a limit order when there are multiple orders in the book.
 */
TEST(VectorOrderBookTest, BookShouldCancelOrders2)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create GTC orders on the same side of the book.
    Order order1 = Order::bidLimit(OrderType::GoodTillCancel, 200, 100, 1, 1, 1);
    Order order2 = Order::bidLimit(OrderType::GoodTillCancel, 200, 100, 2, 2, 1);
    Order order3 = Order::bidLimit(OrderType::GoodTillCancel, 200, 100, 3, 3, 1);
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
    auto rejection_event1 = handler.rejection_events.back();
    handler.rejection_events.pop_back();
    ASSERT_EQ(2, rejection_event1.order_id);
    ASSERT_EQ(2, rejection_event1.user_id);
    ASSERT_EQ(200, rejection_event1.quantity_rejected);
    ASSERT_EQ(1, rejection_event1.symbol_id);
    // Order should not have been traded, so no other messages should have been sent.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should handle a single match between GTC limit orders where both orders are fully executed.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders1)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create GTC orders on different sides of the book, with matchable price levels.
    Order order1 = Order::bidLimit(OrderType::GoodTillCancel, 100, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 100, 100, 2, 2, 1);
    // Place new order.
    book.placeOrder(order1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(1));
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    // Place a second order at the same price level and same quantity as the first order.
    // Both orders should be completely filled.
    book.placeOrder(order2);
    // Second order should have never been inserted into the book.
    ASSERT_FALSE(book.hasOrder(2));
    // First order should have been removed.
    ASSERT_FALSE(book.hasOrder(1));
    // Check the receiver for result message.
    auto event1 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(event1.order_id, 2);
    ASSERT_EQ(event1.user_id, 2);
    ASSERT_EQ(event1.order_price, 100);
    ASSERT_EQ(event1.order_quantity, 100);
    auto event2 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(event2.order_id, 1);
    ASSERT_EQ(event2.user_id, 1);
    ASSERT_EQ(event2.order_price, 100);
    ASSERT_EQ(event2.order_quantity, 100);
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should be able to handle a single match between GTC limit orders where one order is fully executed and
 * the other other is only partially executed.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders2)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create GTC orders on different sides of the book, with matchable price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 90, 100, 1, 1, 1);
    Order order2 = Order::bidLimit(OrderType::GoodTillCancel, 100, 100, 2, 2, 1);
    // Place new GTC order.
    book.placeOrder(order1);
    // Book should now contain the first order since it has no match.
    ASSERT_TRUE(book.hasOrder(1));
    // Get the first order and check that it is unmodified.
    ASSERT_EQ(book.getOrder(1).quantity_executed, 0);
    // Place a second GTC order at the same price level but greater quantity than the first order.
    // First order placed should be filled, second order placed should be partially filled.
    book.placeOrder(order2);
    // Second order should have been inserted into the book since it was unfilled.
    ASSERT_TRUE(book.hasOrder(2));
    ASSERT_EQ(book.getOrder(2).quantity_executed, 90);
    // First order should have been removed from the book.
    ASSERT_FALSE(book.hasOrder(1));
    // There should be a message indicating that the first order was executed.
    auto execution_event1 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event1.order_id, 1);
    ASSERT_EQ(execution_event1.user_id, 1);
    ASSERT_EQ(execution_event1.order_quantity, 90);
    ASSERT_EQ(execution_event1.order_price, 100);
    // There should be a message indicating that the second order was traded with the first
    auto trade_event1 = handler.trade_events.back();
    handler.trade_events.pop_back();
    ASSERT_EQ(trade_event1.order_id, 2);
    ASSERT_EQ(trade_event1.user_id, 2);
    ASSERT_EQ(trade_event1.matched_order_id, 1);
    ASSERT_EQ(trade_event1.matched_order_price, 100);
    ASSERT_EQ(trade_event1.quantity, 90);
    // There should not be any other messages.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should be able to handle multiple matches between GTC limit orders - a GTC order is placed that matches with
 * multiple orders at different price levels.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders3)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create GTC orders on different sides of the book with multiple matchable price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 90, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 200, 120, 2, 2, 1);
    Order order3 = Order::bidLimit(OrderType::GoodTillCancel, 200, 200, 3, 3, 1);
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
    // There should be a message indicating that the second order was traded with the third order.
    auto trade_event1 = handler.trade_events.back();
    handler.trade_events.pop_back();
    ASSERT_EQ(trade_event1.order_id, 2);
    ASSERT_EQ(trade_event1.user_id, 2);
    ASSERT_EQ(trade_event1.matched_order_id, 3);
    ASSERT_EQ(trade_event1.quantity, 200 - 90);
    ASSERT_EQ(trade_event1.order_price, 120);
    ASSERT_EQ(trade_event1.matched_order_price, 200);
    // There should be a message indicating that the third order was traded with the first order.
    auto trade_event2 = handler.trade_events.back();
    handler.trade_events.pop_back();
    ASSERT_EQ(trade_event2.order_id, 3);
    ASSERT_EQ(trade_event2.user_id, 3);
    ASSERT_EQ(trade_event2.matched_order_id, 1);
    ASSERT_EQ(trade_event2.quantity, 90);
    ASSERT_EQ(trade_event2.order_price, 200);
    ASSERT_EQ(trade_event2.matched_order_price, 100);
    // There should be a message indicating that the third order was executed.
    auto execution_event1 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event1.order_id, 3);
    ASSERT_EQ(execution_event1.user_id, 3);
    ASSERT_EQ(execution_event1.order_quantity, 200);
    ASSERT_EQ(execution_event1.order_price, 200);
    // There should be a message indicating that the first order was executed.
    auto execution_event3 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event3.order_id, 1);
    ASSERT_EQ(execution_event3.user_id, 1);
    ASSERT_EQ(execution_event3.order_quantity, 90);
    ASSERT_EQ(execution_event3.order_price, 100);
    // There should be no other messages.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should be able to handle IOC orders that are matched with GTC limit orders at multiple
 * price levels and are fully filled.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders4)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create GTC orders on same side of the book with different price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 90, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 100, 110, 2, 2, 1);
    Order order3 = Order::bidLimit(OrderType::GoodTillCancel, 200, 90, 3, 3, 1);
    // Create IOC order on opposite side of book as GTC ask orders.
    Order order4 = Order::bidLimit(OrderType::ImmediateOrCancel, 190, 150, 4, 4, 1);
    // Create IOC order on opposite side of book as GTC bid orders.
    Order order5 = Order::askLimit(OrderType::ImmediateOrCancel, 200, 80, 5, 5, 1);
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
    // Place a third GTC order.
    book.placeOrder(order3);
    // Book should now contain third order since it has no match.
    ASSERT_TRUE(book.hasOrder(3));
    // PLace the first IOC order - it should match with the first two orders placed.
    book.placeOrder(order4);
    // Third GTC order should still be in the book since it was on the opposite side.
    ASSERT_TRUE(book.hasOrder(3));
    // IOC orders and first two GTC orders should not been in the book.
    ASSERT_FALSE(book.hasOrder(4));
    ASSERT_FALSE(book.hasOrder(2));
    ASSERT_FALSE(book.hasOrder(1));
    // PLace the second IOC order - it should match with the third GTC order placed.
    book.placeOrder(order5);
    // IOC order and third GTC order should have matched and should not be in the book.
    ASSERT_FALSE(book.hasOrder(5));
    ASSERT_FALSE(book.hasOrder(3));

    // Notification that second IOC order was executed.
    auto execution_event1 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event1.order_id, 5);
    ASSERT_EQ(execution_event1.user_id, 5);
    ASSERT_EQ(execution_event1.order_quantity, 200);
    ASSERT_EQ(execution_event1.order_price, 80);
    // Notification that third GTC order was executed.
    auto execution_event2 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event2.order_id, 3);
    ASSERT_EQ(execution_event2.user_id, 3);
    ASSERT_EQ(execution_event2.order_quantity, 200);
    ASSERT_EQ(execution_event2.order_price, 90);
    // Notification that first IOC order was executed.
    auto execution_event3 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event3.order_id, 4);
    ASSERT_EQ(execution_event3.user_id, 4);
    ASSERT_EQ(execution_event3.order_quantity, 190);
    ASSERT_EQ(execution_event3.order_price, 150);
    // Notification that second GTC order was executed.
    auto execution_event4 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event4.order_id, 2);
    ASSERT_EQ(execution_event4.user_id, 2);
    ASSERT_EQ(execution_event4.order_quantity, 100);
    ASSERT_EQ(execution_event4.order_price, 110);
    // Notification that first GTC order was executed.
    auto execution_event5 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event5.order_id, 1);
    ASSERT_EQ(execution_event5.user_id, 1);
    ASSERT_EQ(execution_event5.order_quantity, 90);
    ASSERT_EQ(execution_event5.order_price, 100);
    // Notification that the first GTC order was traded with first IOC order.
    auto trade_event1 = handler.trade_events.back();
    handler.trade_events.pop_back();
    ASSERT_EQ(trade_event1.order_id, 4);
    ASSERT_EQ(trade_event1.user_id, 4);
    ASSERT_EQ(trade_event1.matched_order_id, 1);
    ASSERT_EQ(trade_event1.matched_order_price, 100);
    ASSERT_EQ(trade_event1.quantity, 90);
    // There should not be any other notifications.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should be able to handle an FOK limit order that is fully executable.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders5)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create GTC orders on same side of the book with different price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 90, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 100, 110, 2, 2, 1);
    Order order3 = Order::askLimit(OrderType::GoodTillCancel, 50, 110, 3, 3, 1);
    // Create FOK order on opposite side of book as GTC orders.
    Order order4 = Order::bidLimit(OrderType::FillOrKill, 200, 150, 4, 4, 1);
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
    // There should be a message indicating that the FOK order was executed.
    auto execution_event3 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event3.order_id, 4);
    ASSERT_EQ(execution_event3.user_id, 4);
    ASSERT_EQ(execution_event3.order_quantity, 200);
    ASSERT_EQ(execution_event3.order_price, 150);
    // There should be a message indicating that the second GTC order was executed.
    auto execution_event2 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event2.order_id, 2);
    ASSERT_EQ(execution_event2.user_id, 2);
    ASSERT_EQ(execution_event2.order_quantity, 100);
    ASSERT_EQ(execution_event2.order_price, 110);
    // There should be a message indicating that the first GTC order was executed.
    auto execution_event1 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event1.order_id, 1);
    ASSERT_EQ(execution_event1.user_id, 1);
    ASSERT_EQ(execution_event1.order_quantity, 90);
    ASSERT_EQ(execution_event1.order_price, 100);
    // There should be a message indicating that the FOK order was traded with the third GTC order.
    auto trade_event1 = handler.trade_events.back();
    handler.trade_events.pop_back();
    ASSERT_EQ(trade_event1.order_id, 3);
    ASSERT_EQ(trade_event1.user_id, 3);
    ASSERT_EQ(trade_event1.matched_order_id, 4);
    ASSERT_EQ(trade_event1.matched_order_price, 150);
    ASSERT_EQ(trade_event1.quantity, 200 - 90 - 100);
    // There should be a message indicating that the FOK order was traded with the second GTC order.
    auto trade_event2 = handler.trade_events.back();
    handler.trade_events.pop_back();
    ASSERT_EQ(trade_event2.order_id, 4);
    ASSERT_EQ(trade_event2.user_id, 4);
    ASSERT_EQ(trade_event2.matched_order_id, 2);
    ASSERT_EQ(trade_event2.matched_order_price, 110);
    ASSERT_EQ(trade_event2.quantity, 100);
    // There should be a message indicating that the FOK order was traded with the first GTC order.
    auto trade_event3 = handler.trade_events.back();
    handler.trade_events.pop_back();
    ASSERT_EQ(trade_event3.order_id, 4);
    ASSERT_EQ(trade_event3.user_id, 4);
    ASSERT_EQ(trade_event3.matched_order_id, 1);
    ASSERT_EQ(trade_event3.matched_order_price, 100);
    ASSERT_EQ(trade_event3.quantity, 90);
    // There should not be any other messages.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}

/**
 * Order book should be able to handle a market order that is fully executable.
 */
TEST(VectorOrderBookTest, BookShouldMatchOrders6)
{
    // Make event handler for orderbook.
    DebugEventHandler handler;
    // Create the order book.
    OrderBook::VectorOrderBook book{1, handler};
    // Create GTC orders on same side of the book with different price levels.
    Order order1 = Order::askLimit(OrderType::GoodTillCancel, 90, 100, 1, 1, 1);
    Order order2 = Order::askLimit(OrderType::GoodTillCancel, 100, 110, 2, 2, 1);
    Order order3 = Order::askLimit(OrderType::GoodTillCancel, 50, 110, 3, 3, 1);
    // Create market order on opposite side of book as GTC orders.
    Order order4 = Order::bidMarket(200, 4, 4, 1);
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
    // Place the market order - should match with the previous GTC orders.
    book.placeOrder(order4);
    // Book should not contain the market order or the orders that it was executed with.
    ASSERT_FALSE(book.hasOrder(4));
    ASSERT_FALSE(book.hasOrder(2));
    ASSERT_FALSE(book.hasOrder(1));
    // This order should not have been fully filled and should still be in the book.
    ASSERT_TRUE(book.hasOrder(3));
    ASSERT_EQ(book.getOrder(3).quantity_executed, 200 - 90 - 100);
    // There should be a message indicating that the market order was executed.
    auto execution_event3 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event3.order_id, 4);
    ASSERT_EQ(execution_event3.user_id, 4);
    ASSERT_EQ(execution_event3.order_quantity, 200);
    ASSERT_EQ(execution_event3.order_price, 110);
    // There should be a message indicating that the second GTC order was executed.
    auto execution_event2 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event2.order_id, 2);
    ASSERT_EQ(execution_event2.user_id, 2);
    ASSERT_EQ(execution_event2.order_quantity, 100);
    ASSERT_EQ(execution_event2.order_price, 110);
    // There should be a message indicating that the first GTC order was executed.
    auto execution_event1 = handler.execution_events.back();
    handler.execution_events.pop_back();
    ASSERT_EQ(execution_event1.order_id, 1);
    ASSERT_EQ(execution_event1.user_id, 1);
    ASSERT_EQ(execution_event1.order_quantity, 90);
    ASSERT_EQ(execution_event1.order_price, 100);
    // There should be a message indicating that the market order was traded with the third GTC order.
    auto trade_event1 = handler.trade_events.back();
    handler.trade_events.pop_back();
    ASSERT_EQ(trade_event1.order_id, 3);
    ASSERT_EQ(trade_event1.user_id, 3);
    ASSERT_EQ(trade_event1.matched_order_id, 4);
    ASSERT_EQ(trade_event1.matched_order_price, 110);
    ASSERT_EQ(trade_event1.quantity, 200 - 90 - 100);
    // There should be a message indicating that the market order was traded with the second GTC order.
    auto trade_event2 = handler.trade_events.back();
    handler.trade_events.pop_back();
    ASSERT_EQ(trade_event2.order_id, 4);
    ASSERT_EQ(trade_event2.user_id, 4);
    ASSERT_EQ(trade_event2.matched_order_id, 2);
    ASSERT_EQ(trade_event2.matched_order_price, 110);
    ASSERT_EQ(trade_event2.quantity, 100);
    // There should be a message indicating that the market order was traded with the first GTC order.
    auto trade_event3 = handler.trade_events.back();
    handler.trade_events.pop_back();
    ASSERT_EQ(trade_event3.order_id, 4);
    ASSERT_EQ(trade_event3.user_id, 4);
    ASSERT_EQ(trade_event3.matched_order_id, 1);
    ASSERT_EQ(trade_event3.matched_order_price, 100);
    ASSERT_EQ(trade_event3.quantity, 90);
    // There should not be any other messages.
    ASSERT_TRUE(handler.trade_events.empty());
    ASSERT_TRUE(handler.execution_events.empty());
    ASSERT_TRUE(handler.rejection_events.empty());
}