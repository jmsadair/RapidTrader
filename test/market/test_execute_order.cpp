#include "market_test_fixture.h"
/**
 * Tests Executing an order with a provided quantity but not a price.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork1)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Execution data.
    uint64_t executed_quantity = 100;

    // Execute the order.
    market.executeOrder(symbol_id, id1, executed_quantity);

    event_handler.stop();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    event_handler.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    event_handler.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), executed_quantity);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity1 - executed_quantity);

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests Executing an order with a provided quantity and price.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork2)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Execution data.
    uint64_t executed_quantity = 100;
    uint32_t executed_price = 400;

    // Execute the order.
    market.executeOrder(symbol_id, id1, executed_quantity, executed_price);

    event_handler.stop();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    event_handler.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = event_handler.execute_order_notifications.front();
    event_handler.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), executed_price);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), executed_quantity);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity1 - executed_quantity);

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests Executing an order with a provided quantity such that it must be deleted.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork3)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Execution data.
    uint64_t executed_quantity = 200;

    // Execute the order.
    market.executeOrder(symbol_id, id1, executed_quantity);

    event_handler.stop();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    event_handler.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    event_handler.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), executed_quantity);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that first order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    event_handler.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), executed_quantity);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests Executing an order with a provided quantity and price such that it must be deleted.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork4)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Execution data.
    uint64_t executed_quantity = 200;
    uint64_t executed_price = 300;

    // Execute the order.
    market.executeOrder(symbol_id, id1, executed_quantity, executed_price);

    event_handler.stop();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    event_handler.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(event_handler.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = event_handler.execute_order_notifications.front();
    event_handler.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), executed_price);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), executed_quantity);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that first order was deleted.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    event_handler.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), executed_price);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), executed_quantity);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests trying to execute order with invalid parameters returns error.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork5)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 100;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Invalid execution data.
    uint64_t invalid_executed_quantity = 0;
    uint64_t invalid_executed_price = 0;
    uint64_t invalid_executed_id = 0;
    uint64_t invalid_symbol_id = 2;

    // Execute order with invalid quantity.
    ASSERT_EQ(market.executeOrder(symbol_id, id1, invalid_executed_quantity), ErrorStatus::InvalidQuantity);
    // Execute order with invalid price.
    ASSERT_EQ(market.executeOrder(symbol_id, id1, quantity1, invalid_executed_price), ErrorStatus::InvalidPrice);
    // Execute order with invalid ID - does not exist.
    ASSERT_EQ(market.executeOrder(symbol_id, invalid_executed_id, quantity1, price1), ErrorStatus::OrderDoesNotExist);
    // Executed order with invalid symbol ID - does not exist.
    ASSERT_EQ(market.executeOrder(invalid_symbol_id, id1, quantity1, price1), ErrorStatus::SymbolDoesNotExist);

    // Symbol data.
    uint32_t new_symbol_id = 2;
    std::string new_symbol_name = "MSFT";

    // Add the symbol but not the orderbook.
    market.addSymbol(new_symbol_id, new_symbol_name);

    // Execute with invalid orderbook ID - does not exist.
    // Executed order with invalid symbol ID - does not exist.
    ASSERT_EQ(market.executeOrder(invalid_symbol_id, id1, quantity1, price1), ErrorStatus::OrderBookDoesNotExist);

    event_handler.stop();

    // Check that first order was added.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    event_handler.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(new_symbol_id));
    ASSERT_FALSE(event_handler.add_symbol_notifications.empty());
    ASSERT_EQ(event_handler.add_symbol_notifications.front().symbol_id, new_symbol_id);
    ASSERT_EQ(event_handler.add_symbol_notifications.front().name, new_symbol_name);
    event_handler.add_symbol_notifications.pop();

    ASSERT_TRUE(event_handler.empty());
}