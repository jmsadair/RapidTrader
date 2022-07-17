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

    notification_processor.shutdown();

    ASSERT_TRUE(notification_processor.empty());
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

    notification_processor.shutdown();

    // Check that the symbol was added.
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
    notification_processor.add_symbol_notifications.pop();

    ASSERT_TRUE(notification_processor.empty());
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

    notification_processor.shutdown();

    // Check that order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &notification = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(notification.order, order1);

    ASSERT_TRUE(notification_processor.empty());
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

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that first order was executed - should be completely filled.
    // Note that bid orders are always processed first.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity2 - quantity1);

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
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

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification3 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification3.order, order3);

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity3 - quantity1);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), 0);

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that the second order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    // Check that the third was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification3 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
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

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity2 - quantity1);

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that the second order was deleted since it could not be filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), quantity2 - quantity1);

    ASSERT_TRUE(notification_processor.empty());
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

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification3 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification3.order, order3);

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity3 - quantity1);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), quantity2 - (quantity3 - quantity1));

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that the third was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
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

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification3 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification3.order, order3);

    // Check that the third was deleted since it could not be filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order, order3);

    ASSERT_TRUE(notification_processor.empty());
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

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification3 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification3.order, order3);

    // Check that first order was executed - should be completely filled.
    // Note that bid orders are always processed first.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity3 - quantity1);

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), quantity3 - quantity2 - quantity1);

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that the second order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    // Check that third order was deleted since it could not be filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification3 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), quantity3 - quantity2 - quantity1);

    ASSERT_TRUE(notification_processor.empty());
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

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity1 - quantity2);

    // Check that the second order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding stop IOC order that is activated when it is added to the book.
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

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification3 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification3.order, order3);

    // Check that third order was updated to be a market order since it was activated.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification1 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(update_order_notification1.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification1.order.getTimeInForce(), tof3);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity3);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity3);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity2 - quantity1);

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), quantity2 - quantity1 - quantity3);

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that third order was deleted since it was converted to a market order.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding stop IOC order that is activated after new limit order is added to the book.
 */
TEST_F(MarketTest, AddIocStopOrder2)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 100;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::IOC;
    uint64_t quantity2 = 25;
    uint64_t price2 = 350;
    uint64_t id2 = 2;
    Order order2 = Order::stopBidOrder(id2, symbol_id, price2, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderTimeInForce tof3 = OrderTimeInForce::GTC;
    uint64_t quantity3 = 50;
    uint64_t price3 = 355;
    uint64_t id3 = 3;
    Order order3 = Order::limitBidOrder(id3, symbol_id, price3, quantity3, tof3);

    // Add the order.
    market.addOrder(order3);

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification3 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification3.order, order3);

    // Check that second order was updated to be a market order since it was activated.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification1 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(update_order_notification1.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification1.order.getTimeInForce(), tof2);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity2);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity2);

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity1 - quantity3);

    // Check second order was executed - should be fully filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), quantity1 - quantity2 - quantity3);

    // Check that the third order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that second order was deleted since it was a market order.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding multiple stop IOC orders that are activated after new limit order is added to the book.
 */
TEST_F(MarketTest, AddIocStopOrder3)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 800;
    uint64_t price1 = 321;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::IOC;
    uint64_t quantity2 = 100;
    uint64_t price2 = 320;
    uint64_t id2 = 2;
    Order order2 = Order::stopBidOrder(id2, symbol_id, price2, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderTimeInForce tof3 = OrderTimeInForce::IOC;
    uint64_t quantity3 = 200;
    uint64_t price3 = 310;
    uint64_t id3 = 3;
    Order order3 = Order::stopBidOrder(id3, symbol_id, price3, quantity3, tof3);

    // Add the order.
    market.addOrder(order3);

    // Order to add.
    OrderTimeInForce tof4 = OrderTimeInForce::IOC;
    uint64_t quantity4 = 220;
    uint64_t price4 = 305;
    uint64_t id4 = 4;
    Order order4 = Order::stopBidOrder(id4, symbol_id, price4, quantity4, tof4);

    // Add the order.
    market.addOrder(order4);

    // Order to add.
    OrderTimeInForce tof5 = OrderTimeInForce::GTC;
    uint64_t quantity5 = 50;
    uint64_t price5 = 330;
    uint64_t id5 = 5;
    Order order5 = Order::limitBidOrder(id5, symbol_id, price5, quantity5, tof5);

    // Add the order.
    market.addOrder(order5);

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification3 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification3.order, order3);

    // Check that fourth order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification4 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification4.order, order4);

    // Check that fifth order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification5 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification5.order, order5);

    // Check that fourth order was updated to be a market order since it was activated.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification1 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id4);
    ASSERT_EQ(update_order_notification1.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification1.order.getTimeInForce(), tof4);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity4);

    // Check that third order was updated to be a market order since it was activated.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification2 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(update_order_notification2.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification2.order.getTimeInForce(), tof3);
    ASSERT_EQ(update_order_notification2.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification2.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification2.order.getQuantity(), quantity3);
    ASSERT_EQ(update_order_notification2.order.getOpenQuantity(), quantity3);

    // Check that second order was updated to be a market order since it was activated.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification3 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification3.order.getOrderID(), id2);
    ASSERT_EQ(update_order_notification3.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification3.order.getTimeInForce(), tof2);
    ASSERT_EQ(update_order_notification3.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification3.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification3.order.getQuantity(), quantity2);
    ASSERT_EQ(update_order_notification3.order.getOpenQuantity(), quantity2);

    // Check that fifth order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id5);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity1 - quantity5);

    // Check that fourth order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id4);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), quantity1 - quantity5 - quantity4);

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification5 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification5.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification5.order.getOpenQuantity(), 0);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification6 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification6.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification6.order.getOpenQuantity(), quantity1 - quantity5 - quantity4 - quantity3);

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification7 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification7.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification7.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification7.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification7.order.getOpenQuantity(), 0);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification8 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification8.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification8.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification8.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification8.order.getOpenQuantity(), quantity1 - quantity5 - quantity4 - quantity3 - quantity2);

    // Check that fifth order was deleted since it was filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id5);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that the fourth order was deleted since it was a market order.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id4);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    // Check that third order was deleted since it was a market order.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification3 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);

    // Check that second order was deleted since it was a market order.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification4 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification4.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification4.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding stop limit GTC order that is activated when it is added to the book.
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

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification3 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification3.order, order3);

    // Check that third order was updated to be a limit order since it was activated.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification1 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(update_order_notification1.order.getType(), OrderType::Limit);
    ASSERT_EQ(update_order_notification1.order.getTimeInForce(), tof3);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity3);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity3);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity2 - quantity1);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding trailing stop IOC order to the book that is activated after a trade.
 */
TEST_F(MarketTest, AddGIocTrailingStopOrder1)
{
    // The stop price of the trailing stop order below is originally 100.
    // Price goes up by 10 so stop price becomes 110.
    uint64_t expected_stop_price = 110;

    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::IOC;
    uint64_t quantity1 = 50;
    uint64_t price1 = 100;
    uint64_t id1 = 1;
    Order order1 = Order::trailingStopAskOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 100;
    uint64_t price2 = 170;
    uint64_t id2 = 2;
    Order order2 = Order::limitAskOrder(id2, symbol_id, price2, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderTimeInForce tof3 = OrderTimeInForce::GTC;
    uint64_t quantity3 = 100;
    uint64_t price3 = 160;
    uint64_t id3 = 3;
    Order order3 = Order::limitAskOrder(id3, symbol_id, price3, quantity3, tof3);

    // Add the order.
    market.addOrder(order3);

    // Order to add.
    OrderTimeInForce tof4 = OrderTimeInForce::GTC;
    uint64_t quantity4 = 200;
    uint64_t price4 = 170;
    uint64_t id4 = 4;
    Order order4 = Order::limitBidOrder(id4, symbol_id, price4, quantity4, tof4);

    // Add the order.
    // This order and the previous two orders will match at 160 and 170, bringing the last traded price to 170.
    // The stop price of the trailing stop should then increase by 10.
    market.addOrder(order4);

    // Order to add.
    OrderTimeInForce tof5 = OrderTimeInForce::GTC;
    uint64_t quantity5 = 175;
    uint64_t price5 = 105;
    uint64_t id5 = 5;
    Order order5 = Order::limitBidOrder(id5, symbol_id, price5, quantity5, tof5);

    // Add the order.
    market.addOrder(order5);

    // Order to add.
    OrderTimeInForce tof6 = OrderTimeInForce::GTC;
    uint64_t quantity6 = 100;
    uint64_t price6 = 100;
    uint64_t id6 = 6;
    Order order6 = Order::limitAskOrder(id6, symbol_id, price6, quantity6, tof6);

    // Add the order.
    // This order and the previous order should match at 105, bringing the last traded price to 105.
    // This should activate the trailing stop since it has a stop price of 110.
    market.addOrder(order6);

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification3 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification3.order, order3);

    // Check that fourth order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification4 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification4.order, order4);

    // Check that fifth order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification5 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification5.order, order5);

    // Check that sixth order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification6 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification6.order, order6);

    // Check that first order's stop price was adjusted.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification1 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(update_order_notification1.order.getType(), OrderType::TrailingStop);
    ASSERT_EQ(update_order_notification1.order.getTimeInForce(), tof1);
    ASSERT_EQ(update_order_notification1.order.getStopPrice(), expected_stop_price);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity1);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity1);

    // Check that first order was converted to a market order.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification2 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(update_order_notification2.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification2.order.getTimeInForce(), tof1);
    ASSERT_EQ(update_order_notification2.order.getStopPrice(), expected_stop_price);
    ASSERT_EQ(update_order_notification2.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification2.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification2.order.getQuantity(), quantity1);
    ASSERT_EQ(update_order_notification2.order.getOpenQuantity(), quantity1);

    // Check that fourth order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id4);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity4 - quantity3);

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);

    // Check that fourth order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id4);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), 0);

    // Check that fifth order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification5 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification5.order.getOrderID(), id5);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedQuantity(), quantity6);
    ASSERT_EQ(execute_order_notification5.order.getOpenQuantity(), quantity5 - quantity6);

    // Check that sixth order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification6 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification6.order.getOrderID(), id6);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedQuantity(), quantity6);
    ASSERT_EQ(execute_order_notification6.order.getOpenQuantity(), 0);

    // Check that fifth order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification7 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification7.order.getOrderID(), id5);
    ASSERT_EQ(execute_order_notification7.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification7.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification7.order.getOpenQuantity(), quantity5 - quantity6 - quantity1);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification8 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification8.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification8.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification8.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification8.order.getOpenQuantity(), 0);

    // Check that the third order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that the second order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    // Check that the fourth order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification3 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id4);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity4 - quantity3);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);

    // Check that the sixth order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification4 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification4.order.getOrderID(), id6);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedQuantity(), quantity6);
    ASSERT_EQ(delete_order_notification4.order.getOpenQuantity(), 0);

    // Check that the first order was deleted since it became a market order.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification5 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification5.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification5.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(delete_order_notification5.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification5.order.getOpenQuantity(), 0);
}

/**
 * Tests adding trailing stop IOC order to the book that is activated after another
 * stop order was traded.
 */
TEST_F(MarketTest, AddIocTrailingStopOrder2)
{
    uint64_t expected_stop_price1 = 95;
    uint64_t expected_stop_price2 = 100;

    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::IOC;
    uint64_t quantity1 = 50;
    uint64_t price1 = 95;
    uint64_t id1 = 1;
    Order order1 = Order::trailingStopBidOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::IOC;
    uint64_t quantity2 = 100;
    uint64_t price2 = 100;
    uint64_t id2 = 2;
    Order order2 = Order::trailingStopBidOrder(id2, symbol_id, price2, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderTimeInForce tof3 = OrderTimeInForce::GTC;
    uint64_t quantity3 = 100;
    uint64_t price3 = 95;
    uint64_t id3 = 3;
    Order order3 = Order::limitAskOrder(id3, symbol_id, price3, quantity3, tof3);

    // Add the order.
    market.addOrder(order3);

    // Order to add.
    OrderTimeInForce tof4 = OrderTimeInForce::GTC;
    uint64_t quantity4 = 100;
    uint64_t price4 = 90;
    uint64_t id4 = 4;
    Order order4 = Order::limitAskOrder(id4, symbol_id, price4, quantity4, tof4);

    // Add the order.
    market.addOrder(order4);

    // Order to add.
    OrderTimeInForce tof5 = OrderTimeInForce::GTC;
    uint64_t quantity5 = 700;
    uint64_t price5 = 100;
    uint64_t id5 = 5;
    Order order5 = Order::limitAskOrder(id5, symbol_id, price5, quantity5, tof5);

    // Add the order.
    market.addOrder(order5);

    // Order to add - matches with ask order at 90 and then 95 activating stop orders.
    OrderTimeInForce tof6 = OrderTimeInForce::GTC;
    uint64_t quantity6 = 200;
    uint64_t price6 = 100;
    uint64_t id6 = 6;
    Order order6 = Order::limitBidOrder(id6, symbol_id, price6, quantity6, tof6);

    // Add the order.
    // This order and the previous order should match at 105, bringing the last traded price to 105.
    // This should activate the trailing stop since it has a stop price of 110.
    market.addOrder(order6);

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that third order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification3 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification3.order, order3);

    // Check that fourth order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification4 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification4.order, order4);

    // Check that fifth order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification5 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification5.order, order5);

    // Check that sixth order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification6 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification6.order, order6);

    // Check that first order was converted to a market order.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification1 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(update_order_notification1.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification1.order.getTimeInForce(), tof1);
    ASSERT_EQ(update_order_notification1.order.getStopPrice(), expected_stop_price1);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity1);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity1);

    // Check that second order was converted to a market order.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification2 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(update_order_notification2.order.getType(), OrderType::Market);
    ASSERT_EQ(update_order_notification2.order.getTimeInForce(), tof2);
    ASSERT_EQ(update_order_notification2.order.getStopPrice(), expected_stop_price2);
    ASSERT_EQ(update_order_notification2.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification2.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification2.order.getQuantity(), quantity2);
    ASSERT_EQ(update_order_notification2.order.getOpenQuantity(), quantity2);

    // Check that sixth order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id6);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price4);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity6 - quantity4);

    // Check that fourth order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id4);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price4);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);

    // Check that sixth order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id6);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), 0);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification5 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification5.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification5.order.getOpenQuantity(), 0);

    // Check that fifth order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification6 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification6.order.getOrderID(), id5);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification6.order.getOpenQuantity(), quantity5 - quantity1);

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification7 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification7.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification7.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification7.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification7.order.getOpenQuantity(), 0);

    // Check that fifth order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification8 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification8.order.getOrderID(), id5);
    ASSERT_EQ(execute_order_notification8.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification8.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification8.order.getOpenQuantity(), quantity5 - quantity2 - quantity1);

    // Check that the fourth order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id4);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price4);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that the third order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    // Check that the sixth order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification3 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id6);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);

    // Check that the first order was deleted since it was a market order.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification4 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification4.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification4.order.getOpenQuantity(), 0);

    // Check that the second order was deleted since it became a market order.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification5 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification5.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification5.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(delete_order_notification5.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification5.order.getOpenQuantity(), 0);
}