#include "market_test_fixture.h"

/**
 * Tests replacing an order such that it does not result in matching.
 */
TEST_F(MarketTest, ReplaceOrderShouldWork1)
{
    // Order to add.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 1000;
    uint32_t price1 = 1500;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // New order data.
    uint64_t new_order_id = 2;
    uint32_t new_order_price = 1200;

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
    ASSERT_EQ(add_order_notification2.order.getSide(), side1);
    ASSERT_EQ(add_order_notification2.order.getAction(), action1);
    ASSERT_EQ(add_order_notification2.order.getType(), type1);
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
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 100;
    uint32_t price1 = 1500;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Bid;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 1000;
    uint32_t price2 = 1200;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderAction action3 = OrderAction::Limit;
    OrderSide side3 = OrderSide::Ask;
    OrderType type3 = OrderType::GoodTillCancel;
    uint32_t quantity3 = 500;
    uint32_t price3 = 2000;
    uint64_t id3 = 3;
    Order order3{action3, side3, type3, symbol_id, price3, quantity3, id3};

    // Add the order.
    market.addOrder(order3);

    // New order data.
    uint64_t new_order_id = 4;
    uint32_t new_order_price = 900;

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
    ASSERT_EQ(add_order_notification4.order.getSide(), side3);
    ASSERT_EQ(add_order_notification4.order.getAction(), action3);
    ASSERT_EQ(add_order_notification4.order.getType(), type3);
    ASSERT_EQ(add_order_notification4.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(add_order_notification4.order.getLastExecutedPrice(), 0);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), new_order_price);
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
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), new_order_price);
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
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), new_order_price);
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
