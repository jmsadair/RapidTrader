#include "market_test_fixture.h"

/**
 * Tests adding order with invalid symbol.
 */
TEST_F(MarketTest, AddInvalidOrder1)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 100;
    uint64_t id1 = 1;
    uint32_t symbol_id1 = 10;
    Order order1 = Order::limitBidOrder(id1, symbol_id1, price1, quantity1, tof1);

    // Add the order.
    ASSERT_EQ(market.addOrder(order1), ErrorStatus::SymbolDoesNotExist);

    event_handler.stop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding order with invalid orderbook - symbol exists but orderbook does not.
 */
TEST_F(MarketTest, AddInvalidOrder2)
{
    // Symbol added.
    std::string symbol_name = "GOOG";
    uint32_t symbol_id = 2;

    // Add the symbol;
    market.addSymbol(symbol_id, symbol_name);

    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 100;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    ASSERT_EQ(market.addOrder(order1), ErrorStatus::OrderBookDoesNotExist);

    event_handler.stop();

    // Check that the symbol was added.
    ASSERT_FALSE(event_handler.add_symbol_notifications.empty());
    event_handler.add_symbol_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding GTC limit order to empty orderbook.
 */
TEST_F(MarketTest, AddGtcLimitOrder1)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    event_handler.stop();

    // Check that order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &notification = event_handler.add_order_notifications.front();
    ASSERT_EQ(notification.order, order1);
    event_handler.add_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding GTC limit orders that are matchable to an orderbook.
 */
TEST_F(MarketTest, AddGtcLimitOrder2)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 500;
    uint64_t price2 = 200;
    uint64_t id2 = 2;
    Order order2 = Order::limitAskOrder(id2, symbol_id, price2, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    event_handler.stop();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification1.order, order1);
    event_handler.add_order_notifications.pop();

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification2 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification2.order, order2);
    event_handler.add_order_notifications.pop();

    // Check that first order was executed - should be completely filled.
    // Note that bid orders are always processed first.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity2 - quantity1);
    event_handler.execute_order_notifications.pop();

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding limit IOC order that is not able to be completely filled to an orderbook.
 */
TEST_F(MarketTest, AddIocLimitOrder1)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 100;
    uint64_t price2 = 400;
    uint64_t id2 = 2;
    Order order2 = Order::limitAskOrder(id2, symbol_id, price2, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderTimeInForce tof3 = OrderTimeInForce::IOC;
    uint64_t quantity3 = 300;
    uint64_t price3 = 450;
    uint64_t id3 = 3;
    Order order3 = Order::limitBidOrder(id3, symbol_id, price3, quantity3, tof3);

    // Add the order.
    market.addOrder(order3);

    event_handler.stop();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification1.order, order1);
    event_handler.add_order_notifications.pop();

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification2 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification2.order, order2);
    event_handler.add_order_notifications.pop();

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification3 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification3.order, order3);
    event_handler.add_order_notifications.pop();

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity3 - quantity1);
    event_handler.execute_order_notifications.pop();

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the second order was deleted since it was completely filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification2 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the third was deleted since it was completely filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification3 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding limit IOC order that is not able to be completely filled to an orderbook.
 */
TEST_F(MarketTest, AddIocLimitOrder2)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::IOC;
    uint64_t quantity2 = 300;
    uint64_t price2 = 300;
    uint64_t id2 = 2;
    Order order2 = Order::limitAskOrder(id2, symbol_id, price2, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    event_handler.stop();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification1.order, order1);
    event_handler.add_order_notifications.pop();

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification2 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification2.order, order2);
    event_handler.add_order_notifications.pop();

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity2 - quantity1);
    event_handler.execute_order_notifications.pop();

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the second order was deleted since it could not be filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification2 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), quantity2 - quantity1);
    event_handler.delete_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding limit FOK order that is able to be completely filled to an orderbook.
 */
TEST_F(MarketTest, AddFokLimitOrder1)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 100;
    uint64_t price2 = 400;
    uint64_t id2 = 2;
    Order order2 = Order::limitAskOrder(id2, symbol_id, price2, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderTimeInForce tof3 = OrderTimeInForce::FOK;
    uint64_t quantity3 = 250;
    uint64_t price3 = 450;
    uint64_t id3 = 3;
    Order order3 = Order::limitBidOrder(id3, symbol_id, price3, quantity3, tof3);

    // Add the order.
    market.addOrder(order3);

    event_handler.stop();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification1.order, order1);
    event_handler.add_order_notifications.pop();

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification2 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification2.order, order2);
    event_handler.add_order_notifications.pop();

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification3 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification3.order, order3);
    event_handler.add_order_notifications.pop();

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity3 - quantity1);
    event_handler.execute_order_notifications.pop();

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), quantity2 - (quantity3 - quantity1));
    event_handler.execute_order_notifications.pop();

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the third was deleted since it was completely filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification2 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding limit FOK order that is not able to be completely filled to an orderbook.
 */
TEST_F(MarketTest, AddFokLimitOrder2)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 100;
    uint64_t price2 = 400;
    uint64_t id2 = 2;
    Order order2 = Order::limitBidOrder(id2, symbol_id, price2, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderTimeInForce tof3 = OrderTimeInForce::FOK;
    uint64_t quantity3 = 1000;
    uint64_t price3 = 450;
    uint64_t id3 = 3;
    Order order3 = Order::limitAskOrder(id3, symbol_id, price3, quantity3, tof3);

    // Add the order.
    market.addOrder(order3);

    event_handler.stop();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification1.order, order1);
    event_handler.add_order_notifications.pop();

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification2 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification2.order, order2);
    event_handler.add_order_notifications.pop();

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification3 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification3.order, order3);
    event_handler.add_order_notifications.pop();

    // Check that the third was deleted since it could not be filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification2 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification2.order, order3);
    event_handler.delete_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());

}

/**
 * Tests adding market IOC order that is not able to be completely filled to an orderbook.
 */
TEST_F(MarketTest, AddIocMarketOrder1)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 100;
    uint64_t price2 = 250;
    uint64_t id2 = 2;
    Order order2 = Order::limitBidOrder(id2, symbol_id, price2, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderTimeInForce tof3 = OrderTimeInForce::IOC;
    uint64_t quantity3 = 500;
    uint64_t id3 = 3;
    Order order3 = Order::marketAskOrder(id3, symbol_id, quantity3, tof3);

    // Add the order.
    market.addOrder(order3);

    event_handler.stop();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification1.order, order1);
    event_handler.add_order_notifications.pop();

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification2 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification2.order, order2);
    event_handler.add_order_notifications.pop();

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification3 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification3.order, order3);
    event_handler.add_order_notifications.pop();

    // Check that first order was executed - should be completely filled.
    // Note that bid orders are always processed first.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity3 - quantity1);
    event_handler.execute_order_notifications.pop();

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), quantity3 - quantity2 - quantity1);
    event_handler.execute_order_notifications.pop();

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the second order was deleted since it was completely filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification2 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that third order was deleted since it could not be filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification3 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), quantity3 - quantity2 - quantity1);
    event_handler.delete_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding market IOC order that is able to be completely filled to an orderbook.
 */
TEST_F(MarketTest, AddIocMarketOrder2)
{
    // Add the symbol.
    market.addSymbol(symbol_id, symbol_name);
    // Add the orderbook for the symbol.
    market.addOrderbook(symbol_id);

    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::IOC;
    uint64_t quantity2 = 100;
    uint64_t id2 = 2;
    Order order2 = Order::marketBidOrder(id2, symbol_id, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    event_handler.stop();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification1.order, order1);
    event_handler.add_order_notifications.pop();

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification2 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification2.order, order2);
    event_handler.add_order_notifications.pop();

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity1 - quantity2);
    event_handler.execute_order_notifications.pop();

    // Check that the second order was deleted since it was completely filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding restart IOC order that is activated when it is added to the book.
 */
TEST_F(MarketTest, AddIocStopOrder1)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 900;
    uint64_t price2 = 250;
    uint64_t id2 = 2;
    Order order2 = Order::limitAskOrder(id2, symbol_id, price2, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderTimeInForce tof3 = OrderTimeInForce::IOC;
    uint64_t quantity3 = 500;
    uint64_t price3 = 300;
    uint64_t id3 = 3;
    Order order3 = Order::stopBidOrder(id3, symbol_id, price3, quantity3, tof3);

    // Add the order.
    market.addOrder(order3);

    event_handler.stop();

    // Check that first order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification1.order, order1);
    event_handler.add_order_notifications.pop();

    // Check that second order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification2 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification2.order, order2);
    event_handler.add_order_notifications.pop();

    // Check that third order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification3 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification3.order, order3);
    event_handler.add_order_notifications.pop();

    // Check that third order was updated to be a market order.
    ASSERT_FALSE(event_handler.update_order_notifications.empty());
    OrderUpdated &update_order_notification1 = event_handler.update_order_notifications.front();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(update_order_notification1.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification1.order.getTimeInForce(), tof3);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity3);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity3);
    event_handler.update_order_notifications.pop();

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity2 - quantity1);
    event_handler.execute_order_notifications.pop();

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), quantity2 - quantity1 - quantity3);
    event_handler.execute_order_notifications.pop();

    // Check that the first order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that third order was deleted since it was converted to a market order.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification2 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding restart IOC order that is activated after new limit order is added to the book.
 */
TEST_F(MarketTest, AddIocStopOrder2)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 50;
    uint64_t price1 = 348;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    // Matches with last order - last traded price is now 348.
    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 50;
    uint64_t price2 = 345;
    uint64_t id2 = 2;
    Order order2 = Order::limitAskOrder(id2, symbol_id, price2, quantity2, tof2);
    market.addOrder(order2);

    // Does not activate - last traded price exceeds stop price.
    OrderTimeInForce tof3 = OrderTimeInForce::IOC;
    uint64_t quantity3 = 25;
    uint64_t price3 = 344;
    uint64_t id3 = 3;
    Order order3 = Order::stopAskOrder(id3, symbol_id, price3, quantity3, tof3);
    market.addOrder(order3);

    OrderTimeInForce tof4 = OrderTimeInForce::GTC;
    uint64_t quantity4 = 200;
    uint64_t price4 = 343;
    uint64_t id4 = 4;
    Order order4 = Order::limitBidOrder(id4, symbol_id, price4, quantity4, tof4);
    market.addOrder(order4);

    // Matches with last order - last traded price is now 343.
    // Stop order should activate - last traded price has fallen below stop price.
    OrderTimeInForce tof5 = OrderTimeInForce::GTC;
    uint64_t quantity5 = 50;
    uint64_t price5 = 342;
    uint64_t id5 = 5;
    Order order5 = Order::limitAskOrder(id5, symbol_id, price5, quantity5, tof5);

    market.addOrder(order5);
    event_handler.stop();

    // Check that first order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification1.order, order1);
    event_handler.add_order_notifications.pop();

    // Check that second order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification2 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification2.order, order2);
    event_handler.add_order_notifications.pop();

    // Check that third order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification3 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification3.order, order3);
    event_handler.add_order_notifications.pop();

    // Check that fourth order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification4 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification4.order, order4);
    event_handler.add_order_notifications.pop();

    // Check that fifth order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification5 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification5.order, order5);
    event_handler.add_order_notifications.pop();

    // Check that third order was updated to be a market order since it was activated.
    ASSERT_FALSE(event_handler.update_order_notifications.empty());
    OrderUpdated &update_order_notification1 = event_handler.update_order_notifications.front();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(update_order_notification1.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification1.order.getTimeInForce(), tof3);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity3);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity3);
    ASSERT_EQ(update_order_notification1.order.getStopPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getTrailAmount(), 0);
    event_handler.update_order_notifications.pop();

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check fourth order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id4);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price4);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), quantity4 - quantity5);
    event_handler.execute_order_notifications.pop();

    // Check that fifth order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id5);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price4);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check fourth order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification5 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification5.order.getOrderID(), id4);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedPrice(), price4);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification5.order.getOpenQuantity(), quantity4 - quantity5 - quantity3);
    event_handler.execute_order_notifications.pop();

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification6 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification6.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedPrice(), price4);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification6.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that the first order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that second order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification2 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that fifth order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification3 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id5);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price4);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that third order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification4 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification4.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedPrice(), price4);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification4.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding multiple restart IOC orders that are activated after new limit order is added to the book.
 */
TEST_F(MarketTest, AddIocStopOrder3)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 25;
    uint64_t price1 = 321;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    // Matches with last order - last traded price is now 321.
    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 25;
    uint64_t price2 = 321;
    uint64_t id2 = 2;
    Order order2 = Order::limitBidOrder(id2, symbol_id, price2, quantity2, tof2);
    market.addOrder(order2);

    // Does not activate - stop price exceeds last traded price.
    OrderTimeInForce tof3 = OrderTimeInForce::IOC;
    uint64_t quantity3 = 100;
    uint64_t price3 = 322;
    uint64_t id3 = 3;
    Order order3 = Order::stopBidOrder(id3, symbol_id, price3, quantity3, tof3);
    market.addOrder(order3);

    // Does not activate - stop price exceeds last traded price.
    OrderTimeInForce tof4 = OrderTimeInForce::IOC;
    uint64_t quantity4 = 200;
    uint64_t price4 = 322;
    uint64_t id4 = 4;
    Order order4 = Order::stopBidOrder(id4, symbol_id, price4, quantity4, tof4);
    market.addOrder(order4);

    // Does not activate - stop price exceeds last traded price.
    OrderTimeInForce tof5 = OrderTimeInForce::IOC;
    uint64_t quantity5 = 220;
    uint64_t price5 = 323;
    uint64_t id5 = 5;
    Order order5 = Order::stopBidOrder(id5, symbol_id, price5, quantity5, tof5);
    market.addOrder(order5);

    OrderTimeInForce tof6 = OrderTimeInForce::GTC;
    uint64_t quantity6 = 50;
    uint64_t price6 = 324;
    uint64_t id6 = 6;
    Order order6 = Order::limitBidOrder(id6, symbol_id, price6, quantity6, tof6);
    market.addOrder(order6);

    // Matches with last order - last traded price is now 324.
    // Should activate all stop orders.
    OrderTimeInForce tof7 = OrderTimeInForce::GTC;
    uint64_t quantity7 = 700;
    uint64_t price7 = 321;
    uint64_t id7 = 7;
    Order order7 = Order::limitAskOrder(id7, symbol_id, price7, quantity7, tof7);
    market.addOrder(order7);

    event_handler.stop();

    // Check that first order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification1.order, order1);
    event_handler.add_order_notifications.pop();

    // Check that second order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification2 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification2.order, order2);
    event_handler.add_order_notifications.pop();

    // Check that third order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification3 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification3.order, order3);
    event_handler.add_order_notifications.pop();

    // Check that fourth order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification4 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification4.order, order4);
    event_handler.add_order_notifications.pop();

    // Check that fifth order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification5 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification5.order, order5);
    event_handler.add_order_notifications.pop();

    // Check that sixth order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification6 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification6.order, order6);
    event_handler.add_order_notifications.pop();

    // Check that seventh order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification7 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification7.order, order7);
    event_handler.add_order_notifications.pop();

    // Check that third order was updated to be a market order.
    ASSERT_FALSE(event_handler.update_order_notifications.empty());
    OrderUpdated &update_order_notification1 = event_handler.update_order_notifications.front();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(update_order_notification1.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification1.order.getTimeInForce(), tof3);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity3);
    ASSERT_EQ(update_order_notification1.order.getStopPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getTrailAmount(), 0);
    event_handler.update_order_notifications.pop();

    // Check that fourth order was updated to be a market order.
    ASSERT_FALSE(event_handler.update_order_notifications.empty());
    OrderUpdated &update_order_notification2 = event_handler.update_order_notifications.front();
    ASSERT_EQ(update_order_notification2.order.getOrderID(), id4);
    ASSERT_EQ(update_order_notification2.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification2.order.getTimeInForce(), tof4);
    ASSERT_EQ(update_order_notification2.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification2.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification2.order.getQuantity(), quantity4);
    ASSERT_EQ(update_order_notification2.order.getOpenQuantity(), quantity4);
    ASSERT_EQ(update_order_notification2.order.getStopPrice(), 0);
    ASSERT_EQ(update_order_notification2.order.getTrailAmount(), 0);
    event_handler.update_order_notifications.pop();

    // Check that fifth order was updated to be a market order.
    ASSERT_FALSE(event_handler.update_order_notifications.empty());
    OrderUpdated &update_order_notification3 = event_handler.update_order_notifications.front();
    ASSERT_EQ(update_order_notification3.order.getOrderID(), id5);
    ASSERT_EQ(update_order_notification3.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification3.order.getTimeInForce(), tof5);
    ASSERT_EQ(update_order_notification3.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification3.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification3.order.getQuantity(), quantity5);
    ASSERT_EQ(update_order_notification3.order.getOpenQuantity(), quantity5);
    ASSERT_EQ(update_order_notification3.order.getStopPrice(), 0);
    ASSERT_EQ(update_order_notification3.order.getTrailAmount(), 0);
    event_handler.update_order_notifications.pop();

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that sixth order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id6);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price6);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity6);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that seventh order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id7);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price6);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity6);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), quantity7 - quantity6);
    event_handler.execute_order_notifications.pop();

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification5 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification5.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification5.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that seventh order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification6 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification6.order.getOrderID(), id7);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification6.order.getOpenQuantity(), quantity7 - quantity6 - quantity3);
    event_handler.execute_order_notifications.pop();

    // Check that fourth order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification7 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification7.order.getOrderID(), id4);
    ASSERT_EQ(execute_order_notification7.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(execute_order_notification7.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(execute_order_notification7.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that seventh order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification8 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification8.order.getOrderID(), id7);
    ASSERT_EQ(execute_order_notification8.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(execute_order_notification8.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(execute_order_notification8.order.getOpenQuantity(), quantity7 - quantity6 - quantity3 - quantity4);
    event_handler.execute_order_notifications.pop();

    // Check that fifth order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification9 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification9.order.getOrderID(), id5);
    ASSERT_EQ(execute_order_notification9.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(execute_order_notification9.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(execute_order_notification9.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that seventh order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification10 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification10.order.getOrderID(), id7);
    ASSERT_EQ(execute_order_notification10.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(execute_order_notification10.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(execute_order_notification10.order.getOpenQuantity(), quantity7 - quantity6 - quantity3 - quantity4 - quantity5);
    event_handler.execute_order_notifications.pop();

    // Check that first order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the second order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification2 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that sixth order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification3 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id6);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price6);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity6);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that third order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification4 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification4.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification4.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that fourth order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification5 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification5.order.getOrderID(), id4);
    ASSERT_EQ(delete_order_notification5.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(delete_order_notification5.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(delete_order_notification5.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that fifth order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification6 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification6.order.getOrderID(), id5);
    ASSERT_EQ(delete_order_notification6.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(delete_order_notification6.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(delete_order_notification6.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding restart limit GTC order that is activated when it is added to the book.
 */
TEST_F(MarketTest, AddGtcStopLimitOrder1)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 350;
    uint64_t price2 = 400;
    uint64_t id2 = 2;
    Order order2 = Order::limitBidOrder(id2, symbol_id, price2, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderTimeInForce tof3 = OrderTimeInForce::GTC;
    uint64_t quantity3 = 500;
    uint64_t stop_price = 300;
    uint64_t price3 = 500;
    uint64_t id3 = 3;
    Order order3 = Order::stopLimitBidOrder(id3, symbol_id, price3, stop_price, quantity3, tof3);

    // Add the order.
    market.addOrder(order3);

    event_handler.stop();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification1.order, order1);
    event_handler.add_order_notifications.pop();

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification2 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification2.order, order2);
    event_handler.add_order_notifications.pop();

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification3 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification3.order, order3);
    event_handler.add_order_notifications.pop();

    // Check that third order was updated to be a limit order since it was activated.
    ASSERT_FALSE(event_handler.update_order_notifications.empty());
    OrderUpdated &update_order_notification1 = event_handler.update_order_notifications.front();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(update_order_notification1.order.getType(), OrderType::Limit);
    ASSERT_EQ(update_order_notification1.order.getTimeInForce(), tof3);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity3);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity3);
    event_handler.update_order_notifications.pop();

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity2 - quantity1);
    event_handler.execute_order_notifications.pop();

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding trailing stop IOC order to the book that is activated after a trade.
 */
TEST_F(MarketTest, AddIocTrailingStopOrder1)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 170;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    // Matches with last order - last traded price is now 170.
    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 200;
    uint64_t price2 = 170;
    uint64_t id2 = 2;
    Order order2 = Order::limitAskOrder(id2, symbol_id, price2, quantity2, tof2);
    market.addOrder(order2);

    // Last traded price is 170.
    // Trail amount is 1.
    // Inserted into the book with stop price last traded price + trail amount = 169.
    OrderTimeInForce tof3 = OrderTimeInForce::IOC;
    uint64_t quantity3 = 50;
    uint64_t price3 = 165;
    uint64_t trail_amount = 1;
    uint64_t id3 = 3;
    Order order3 = Order::trailingStopAskOrder(id3, symbol_id, price3, trail_amount, quantity3, tof3);
    market.addOrder(order3);

    OrderTimeInForce tof4 = OrderTimeInForce::GTC;
    uint64_t quantity4 = 100;
    uint64_t price4 = 168;
    uint64_t id4 = 4;
    Order order4 = Order::limitAskOrder(id4, symbol_id, price4, quantity4, tof4);
    market.addOrder(order4);

    // Matches with last order - last traded price is now 168.
    // Trailing stop order should be activated.
    OrderTimeInForce tof5 = OrderTimeInForce::GTC;
    uint64_t quantity5 = 200;
    uint64_t price5 = 170;
    uint64_t id5 = 5;
    Order order5 = Order::limitBidOrder(id5, symbol_id, price5, quantity5, tof5);
    market.addOrder(order5);

    event_handler.stop();

    // Check that first order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification1.order, order1);
    event_handler.add_order_notifications.pop();

    // Check that second order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification2 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification2.order, order2);
    event_handler.add_order_notifications.pop();

    // Check that third order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification3 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification3.order, order3);
    event_handler.add_order_notifications.pop();

    // Check that fourth order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification4 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification4.order, order4);
    event_handler.add_order_notifications.pop();

    // Check that fifth order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification5 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification5.order, order5);
    event_handler.add_order_notifications.pop();

    // Check that third order was converted to a market order.
    ASSERT_FALSE(event_handler.update_order_notifications.empty());
    OrderUpdated &update_order_notification1 = event_handler.update_order_notifications.front();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(update_order_notification1.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification1.order.getTimeInForce(), tof3);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity3);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity3);
    ASSERT_EQ(update_order_notification1.order.getStopPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getTrailAmount(), 0);
    event_handler.update_order_notifications.pop();

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that fifth order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id5);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price4);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), quantity5 - quantity4);
    event_handler.execute_order_notifications.pop();

    // Check that fourth order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id4);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price4);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that fifth order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification5 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification5.order.getOrderID(), id5);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification5.order.getOpenQuantity(), quantity5 - quantity4 - quantity3);
    event_handler.execute_order_notifications.pop();

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification6 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification6.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification6.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that the first order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the second order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification2 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the fourth order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification3 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id4);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price4);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the third order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification4 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification4.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification4.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests adding trailing stop IOC order to the book that is activated after another
 * stop order was activated and traded.
 */
TEST_F(MarketTest, AddIocTrailingStopOrder2)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 170;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    // Matches with last order - last traded price is now 170.
    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 200;
    uint64_t price2 = 170;
    uint64_t id2 = 2;
    Order order2 = Order::limitAskOrder(id2, symbol_id, price2, quantity2, tof2);
    market.addOrder(order2);

    // Last traded price is 170.
    // Trail amount is 3.
    // Inserted into book with stop price = last traded price + trail amount = 173.
    OrderTimeInForce tof3 = OrderTimeInForce::IOC;
    uint64_t quantity3 = 50;
    uint64_t price3 = 175;
    uint64_t trail_amount1 = 3;
    uint64_t id3 = 3;
    Order order3 = Order::trailingStopBidOrder(id3, symbol_id, price3, trail_amount1, quantity3, tof3);
    market.addOrder(order3);

    // Last traded price is 170.
    // Trail amount is 2.
    // Inserted into book with stop price = last traded price + trail amount = 172.
    OrderTimeInForce tof4 = OrderTimeInForce::IOC;
    uint64_t quantity4 = 100;
    uint64_t price4 = 175;
    uint64_t trail_amount2 = 2;
    uint64_t id4 = 4;
    Order order4 = Order::trailingStopBidOrder(id4, symbol_id, price4, trail_amount2, quantity4, tof4);
    market.addOrder(order4);

    OrderTimeInForce tof5 = OrderTimeInForce::GTC;
    uint64_t quantity5 = 100;
    uint64_t price5 = 168;
    uint64_t id5 = 5;
    Order order5 = Order::limitAskOrder(id5, symbol_id, price5, quantity5, tof5);
    market.addOrder(order5);

    // Matches with last order - traded price is now 168.
    // Stop price of trailing stops should be adjusted.
    // First trailing stop order should have stop price = last traded price + trail amount = 171.
    // Second trailing stop order should have stop price = last traded price + trail amount = 170.
    uint64_t expected_stop_price1 = 170;
    uint64_t expected_stop_price2 = 171;
    OrderTimeInForce tof6 = OrderTimeInForce::GTC;
    uint64_t quantity6 = 100;
    uint64_t price6 = 170;
    uint64_t id6 = 6;
    Order order6 = Order::limitBidOrder(id6, symbol_id, price6, quantity6, tof6);
    market.addOrder(order6);

    OrderTimeInForce tof7 = OrderTimeInForce::GTC;
    uint64_t quantity7 = 700;
    uint64_t price7 = 172;
    uint64_t id7 = 7;
    Order order7 = Order::limitAskOrder(id7, symbol_id, price7, quantity7, tof7);
    market.addOrder(order7);

    OrderTimeInForce tof8 = OrderTimeInForce::GTC;
    uint64_t quantity8 = 200;
    uint64_t price8 = 170;
    uint64_t id8 = 8;
    Order order8 = Order::limitAskOrder(id8, symbol_id, price8, quantity8, tof8);
    market.addOrder(order8);

    // Matches with last order - last traded price is now 170.
    // This should activate the stop order with stop price 170.
    // That stop order should match with ask order with price 172.
    // This should activate the stop order with price 171.
    OrderTimeInForce tof9 = OrderTimeInForce::GTC;
    uint64_t quantity9 = 200;
    uint64_t price9 = 170;
    uint64_t id9 = 9;
    Order order9 = Order::limitBidOrder(id9, symbol_id, price9, quantity9, tof9);
    market.addOrder(order9);

    event_handler.stop();

    // Check that first order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification1.order, order1);
    event_handler.add_order_notifications.pop();

    // Check that second order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification2 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification2.order, order2);
    event_handler.add_order_notifications.pop();

    // Check that third order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification3 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification3.order, order3);
    event_handler.add_order_notifications.pop();

    // Check that fourth order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification4 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification4.order, order4);
    event_handler.add_order_notifications.pop();

    // Check that fifth order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification5 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification5.order, order5);
    event_handler.add_order_notifications.pop();

    // Check that sixth order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification6 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification6.order, order6);
    event_handler.add_order_notifications.pop();

    // Check that seventh order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification7 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification7.order, order7);
    event_handler.add_order_notifications.pop();

    // Check that eighth order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification8 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification8.order, order8);
    event_handler.add_order_notifications.pop();

    // Check that ninth order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification9 = event_handler.add_order_notifications.front();
    ASSERT_EQ(add_order_notification9.order, order9);
    event_handler.add_order_notifications.pop();

    // Check that fourth order had its stop price adjusted.
    ASSERT_FALSE(event_handler.update_order_notifications.empty());
    OrderUpdated &update_order_notification1 = event_handler.update_order_notifications.front();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id4);
    ASSERT_EQ(update_order_notification1.order.getType(), OrderType::TrailingStop);
    ASSERT_EQ(update_order_notification1.order.getTimeInForce(), tof4);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity4);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity4);
    ASSERT_EQ(update_order_notification1.order.getStopPrice(), expected_stop_price1);
    ASSERT_EQ(update_order_notification1.order.getTrailAmount(), trail_amount2);
    event_handler.update_order_notifications.pop();

    // Check that third order was converted to a market order.
    ASSERT_FALSE(event_handler.update_order_notifications.empty());
    OrderUpdated &update_order_notification2 = event_handler.update_order_notifications.front();
    ASSERT_EQ(update_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(update_order_notification2.order.getType(), OrderType::TrailingStop);
    ASSERT_EQ(update_order_notification2.order.getTimeInForce(), tof3);
    ASSERT_EQ(update_order_notification2.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification2.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification2.order.getQuantity(), quantity3);
    ASSERT_EQ(update_order_notification2.order.getOpenQuantity(), quantity3);
    ASSERT_EQ(update_order_notification2.order.getStopPrice(), expected_stop_price2);
    ASSERT_EQ(update_order_notification2.order.getTrailAmount(), trail_amount1);
    event_handler.update_order_notifications.pop();

    // Check that fourth order was converted to a market order.
    ASSERT_FALSE(event_handler.update_order_notifications.empty());
    OrderUpdated &update_order_notification3 = event_handler.update_order_notifications.front();
    ASSERT_EQ(update_order_notification3.order.getOrderID(), id4);
    ASSERT_EQ(update_order_notification3.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification3.order.getTimeInForce(), tof4);
    ASSERT_EQ(update_order_notification3.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification3.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification3.order.getQuantity(), quantity4);
    ASSERT_EQ(update_order_notification3.order.getOpenQuantity(), quantity4);
    ASSERT_EQ(update_order_notification3.order.getStopPrice(), 0);
    ASSERT_EQ(update_order_notification3.order.getTrailAmount(), 0);
    event_handler.update_order_notifications.pop();

    // Check that third order was converted to a market order.
    ASSERT_FALSE(event_handler.update_order_notifications.empty());
    OrderUpdated &update_order_notification4 = event_handler.update_order_notifications.front();
    ASSERT_EQ(update_order_notification4.order.getOrderID(), id3);
    ASSERT_EQ(update_order_notification4.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification4.order.getTimeInForce(), tof3);
    ASSERT_EQ(update_order_notification4.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification4.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification4.order.getQuantity(), quantity3);
    ASSERT_EQ(update_order_notification4.order.getOpenQuantity(), quantity3);
    ASSERT_EQ(update_order_notification4.order.getStopPrice(), 0);
    ASSERT_EQ(update_order_notification4.order.getTrailAmount(), 0);
    event_handler.update_order_notifications.pop();

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that sixth order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id6);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity6);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that fifth order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id5);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that ninth order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification6 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification6.order.getOrderID(), id9);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedPrice(), price8);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedQuantity(), quantity9);
    ASSERT_EQ(execute_order_notification6.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that eighth order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification7 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification7.order.getOrderID(), id8);
    ASSERT_EQ(execute_order_notification7.order.getLastExecutedPrice(), price8);
    ASSERT_EQ(execute_order_notification7.order.getLastExecutedQuantity(), quantity8);
    ASSERT_EQ(execute_order_notification7.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that fourth order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification8 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification8.order.getOrderID(), id4);
    ASSERT_EQ(execute_order_notification8.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(execute_order_notification8.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(execute_order_notification8.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that seventh order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification9 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification9.order.getOrderID(), id7);
    ASSERT_EQ(execute_order_notification9.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(execute_order_notification9.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(execute_order_notification9.order.getOpenQuantity(), quantity7 - quantity4);
    event_handler.execute_order_notifications.pop();

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification10 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification10.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification10.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(execute_order_notification10.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification10.order.getOpenQuantity(), 0);
    event_handler.execute_order_notifications.pop();

    // Check that seventh order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification11 = event_handler.execute_order_notifications.front();
    ASSERT_EQ(execute_order_notification11.order.getOrderID(), id7);
    ASSERT_EQ(execute_order_notification11.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(execute_order_notification11.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification11.order.getOpenQuantity(), quantity7 - quantity4 - quantity3);
    event_handler.execute_order_notifications.pop();

    // Check that the first order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the second order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification2 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the fifth order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification3 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id5);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the sixth order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification4 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification4.order.getOrderID(), id6);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedQuantity(), quantity6);
    ASSERT_EQ(delete_order_notification4.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the eighth order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification5 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification5.order.getOrderID(), id8);
    ASSERT_EQ(delete_order_notification5.order.getLastExecutedPrice(), price8);
    ASSERT_EQ(delete_order_notification5.order.getLastExecutedQuantity(), quantity8);
    ASSERT_EQ(delete_order_notification5.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the ninth order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification6 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification6.order.getOrderID(), id9);
    ASSERT_EQ(delete_order_notification6.order.getLastExecutedPrice(), price8);
    ASSERT_EQ(delete_order_notification6.order.getLastExecutedQuantity(), quantity9);
    ASSERT_EQ(delete_order_notification6.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the fourth order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification7 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification7.order.getOrderID(), id4);
    ASSERT_EQ(delete_order_notification7.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(delete_order_notification7.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(delete_order_notification7.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    // Check that the third order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification8 = event_handler.delete_order_notifications.front();
    ASSERT_EQ(delete_order_notification8.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification8.order.getLastExecutedPrice(), price7);
    ASSERT_EQ(delete_order_notification8.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification8.order.getOpenQuantity(), 0);
    event_handler.delete_order_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}