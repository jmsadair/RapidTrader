#include <gtest/gtest.h>
#include "market.h"
#include "debug_notification_processor.h"

/**
 * Tests Executing an order with a provided quantity but not a price.
 */
TEST(MarketTest, ExecuteOrderShouldWork1)
{
    DebugNotificationProcessor notification_processor;
    notification_processor.run();
    RapidTrader::Matching::Market market(notification_processor.getSender());

    // Symbol data.
    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";

    // Add the symbol.
    market.addSymbol(symbol_id, symbol_name);
    // Add the orderbook for the symbol.
    market.addOrderbook(symbol_id);

    // Order to add.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Execution data.
    uint64_t executed_quantity = 100;

    // Execute the order.
    market.executeOrder(symbol_id, id1, executed_quantity);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().symbol_id, symbol_id);
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().name, symbol_name);
    notification_processor.add_symbol_notifications.pop();

    // Check that book was added.
    ASSERT_TRUE(market.hasOrderbook(symbol_id));
    ASSERT_EQ(notification_processor.add_book_notifications.front().symbol_id, symbol_id);
    notification_processor.add_book_notifications.pop();

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
TEST(MarketTest, ExecuteOrderShouldWork2)
{
    DebugNotificationProcessor notification_processor;
    notification_processor.run();
    RapidTrader::Matching::Market market(notification_processor.getSender());

    // Symbol data.
    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";

    // Add the symbol.
    market.addSymbol(symbol_id, symbol_name);
    // Add the orderbook for the symbol.
    market.addOrderbook(symbol_id);

    // Order to add.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Execution data.
    uint64_t executed_quantity = 100;
    uint32_t executed_price = 400;

    // Execute the order.
    market.executeOrder(symbol_id, id1, executed_quantity, executed_price);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().symbol_id, symbol_id);
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().name, symbol_name);
    notification_processor.add_symbol_notifications.pop();

    // Check that book was added.
    ASSERT_EQ(notification_processor.add_book_notifications.front().symbol_id, symbol_id);
    notification_processor.add_book_notifications.pop();

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
TEST(MarketTest, ExecuteOrderShouldWork3)
{
    DebugNotificationProcessor notification_processor;
    notification_processor.run();
    RapidTrader::Matching::Market market(notification_processor.getSender());

    // Symbol data.
    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";

    // Add the symbol.
    market.addSymbol(symbol_id, symbol_name);
    // Add the orderbook for the symbol.
    market.addOrderbook(symbol_id);

    // Order to add.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Execution data.
    uint64_t executed_quantity = 200;

    // Execute the order.
    market.executeOrder(symbol_id, id1, executed_quantity);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().symbol_id, symbol_id);
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().name, symbol_name);
    notification_processor.add_symbol_notifications.pop();

    // Check that book was added.
    ASSERT_EQ(notification_processor.add_book_notifications.front().symbol_id, symbol_id);
    notification_processor.add_book_notifications.pop();

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
