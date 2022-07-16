#include "market_test_fixture.h"

/**
 * Tests replacing an order such that it does not result in matching.
 */
TEST_F(MarketTest, ReplaceOrderShouldWork1)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 1000;
    uint64_t price1 = 1500;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // New order data.
    uint64_t new_order_id = 2;
    uint64_t new_order_price = 1200;

    // Replace the order.
    market.replaceOrder(symbol_id, id1, new_order_id, new_order_price);

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that replacement order was added. Order should be identical to original
    // order except its price and ID.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order.getOrderID(), new_order_id);
    ASSERT_EQ(add_order_notification2.order.getSymbolID(), symbol_id);
    ASSERT_EQ(add_order_notification2.order.getPrice(), new_order_price);
    ASSERT_EQ(add_order_notification2.order.getSide(), OrderSide::Bid);
    ASSERT_EQ(add_order_notification2.order.getType(), OrderType::Limit);
    ASSERT_EQ(add_order_notification2.order.getTimeInForce(), OrderTimeInForce::GTC);
    ASSERT_EQ(add_order_notification2.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(add_order_notification2.order.getLastExecutedPrice(), 0);

    // Check that replaced order was deleted - order should be unchanged.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order, order1);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests replacing an order such that it does result in matching.
 */
TEST_F(MarketTest, ReplaceOrderShouldWork2)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 100;
    uint64_t price1 = 1500;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 1000;
    uint64_t price2 = 1200;
    uint64_t id2 = 2;
    Order order2 = Order::limitBidOrder(id2, symbol_id, price2, quantity2, tof2);

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderTimeInForce tof3 = OrderTimeInForce::GTC;
    uint64_t quantity3 = 500;
    uint64_t price3 = 2000;
    uint64_t id3 = 3;
    Order order3 = Order::limitAskOrder(id3, symbol_id, price3, quantity3, tof3);

    // Add the order.
    market.addOrder(order3);

    // New order data.
    uint64_t new_order_id = 4;
    uint64_t new_order_price = 900;

    // Replace the order.
    market.replaceOrder(symbol_id, id3, new_order_id, new_order_price);

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

    // Check that replacement order was added. Order should be identical to original
    // order except its price and ID.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification4 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification4.order.getOrderID(), new_order_id);
    ASSERT_EQ(add_order_notification4.order.getSymbolID(), symbol_id);
    ASSERT_EQ(add_order_notification4.order.getPrice(), new_order_price);
    ASSERT_EQ(add_order_notification4.order.getSide(), OrderSide::Ask);
    ASSERT_EQ(add_order_notification4.order.getType(), OrderType::Limit);
    ASSERT_EQ(add_order_notification4.order.getTimeInForce(), OrderTimeInForce::GTC);
    ASSERT_EQ(add_order_notification4.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(add_order_notification4.order.getLastExecutedPrice(), 0);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that the new replacement order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), new_order_id);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity3 - quantity1);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), quantity2 - (quantity3 - quantity1));

    // Check that the new replacement order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), new_order_id);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), 0);

    // Check that replaced order was deleted - order should be unchanged.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order, order3);

    // Check that first order deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    // Check that the new replacement order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification3 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), new_order_id);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests trying to replace an invalid order returns an error (i.e. the orderbook does not exists,
 * the symbol does not exists, the order does not exist, etc.).
 */
TEST_F(MarketTest, ReplaceOrderShouldWork3)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 100;
    uint64_t price1 = 1500;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);

    // Invalid symbol / orderbook ID - does not exist.
    uint32_t invalid_symbol_id = 0;
    // Invalid price.
    uint64_t invalid_price = 0;
    // Invalid order ID.
    uint64_t invalid_id = 0;

    // Add the order.
    market.addOrder(order1);

    // Replace the order with invalid symbol.
    ASSERT_EQ(market.replaceOrder(invalid_symbol_id, id1, id1, price1), ErrorStatus::SymbolDoesNotExist);
    // Replace the order with invalid replacement order ID.
    ASSERT_EQ(market.replaceOrder(symbol_id, id1, invalid_id, price1), ErrorStatus::InvalidOrderID);
    // Replace the order with invalid price.
    ASSERT_EQ(market.replaceOrder(symbol_id, id1, id1, invalid_price), ErrorStatus::InvalidPrice);
    // Replace the order with ID that does not exist.
    ASSERT_EQ(market.replaceOrder(symbol_id, invalid_id, id1, price1), ErrorStatus::OrderDoesNotExist);

    // Symbol data.
    uint32_t new_symbol_id = 2;
    std::string new_symbol_name = "MSFT";

    // Add the symbol but not the orderbook.
    market.addSymbol(new_symbol_id, new_symbol_name);

    // Execute with invalid orderbook ID - does not exist.
    // Executed order with invalid symbol ID - does not exist.
    ASSERT_EQ(market.replaceOrder(new_symbol_id, id1, id1, price1), ErrorStatus::OrderBookDoesNotExist);

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(new_symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().symbol_id, new_symbol_id);
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().name, new_symbol_name);
    notification_processor.add_symbol_notifications.pop();

    ASSERT_TRUE(notification_processor.empty());
}