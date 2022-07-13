#include "market_test_fixture.h"
/**
 * Tests Executing an order with a provided quantity but not a price.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork1)
{
    // Order to add.
    OrderType type1 = OrderType::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{type1, side1, tof1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Execution data.
    uint64_t executed_quantity = 100;

    // Execute the order.
    market.executeOrder(symbol_id, id1, executed_quantity);

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), executed_quantity);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity1 - executed_quantity);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests Executing an order with a provided quantity and price.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork2)
{
    // Order to add.
    OrderType type1 = OrderType::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{type1, side1, tof1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Execution data.
    uint64_t executed_quantity = 100;
    uint32_t executed_price = 400;

    // Execute the order.
    market.executeOrder(symbol_id, id1, executed_quantity, executed_price);

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), executed_price);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), executed_quantity);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity1 - executed_quantity);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests Executing an order with a provided quantity such that it must be deleted.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork3)
{
    // Order to add.
    OrderType type1 = OrderType::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{type1, side1, tof1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Execution data.
    uint64_t executed_quantity = 200;

    // Execute the order.
    market.executeOrder(symbol_id, id1, executed_quantity);

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), executed_quantity);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that first order was deleted.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), executed_quantity);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests Executing an order with a provided quantity and price such that it must be deleted.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork4)
{
    // Order to add.
    OrderType type1 = OrderType::Limit;
    OrderSide side1 = OrderSide::Bid;
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{type1, side1, tof1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Execution data.
    uint64_t executed_quantity = 200;
    uint64_t executed_price = 300;


    // Execute the order.
    market.executeOrder(symbol_id, id1, executed_quantity, executed_price);

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), executed_price);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), executed_quantity);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that first order was deleted.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), executed_price);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), executed_quantity);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests trying to execute invalid orders.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork5)
{
    // Order to add.
    OrderType type1 = OrderType::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint32_t quantity1 = 200;
    uint32_t price1 = 100;
    uint64_t id1 = 1;
    Order order1{type1, side1, tof1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Execution data - invalid quantity.
    uint64_t executed_quantity = 0;
    uint64_t executed_price = 0;
    uint64_t executed_id = 10;
    uint64_t executed_symbol_id = 2;

    // Execute order - bad quantity.
    ASSERT_EQ(market.executeOrder(symbol_id, id1, executed_quantity), ErrorStatus::InvalidQuantity);
    // Execute order - bad price.
    ASSERT_EQ(market.executeOrder(symbol_id, id1, quantity1, executed_price), ErrorStatus::InvalidPrice);
    // Execute order - bad ID.
    ASSERT_EQ(market.executeOrder(symbol_id, executed_id, quantity1, price1), ErrorStatus::OrderDoesNotExist);
    // Execute order - bad symbol ID.
    ASSERT_EQ(market.executeOrder(executed_symbol_id, id1, quantity1, price1), ErrorStatus::SymbolDoesNotExist);

    notification_processor.shutdown();
}