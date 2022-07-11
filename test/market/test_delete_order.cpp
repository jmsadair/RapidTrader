#include "market_test_fixture.h"

/**
 * Tests deleting an order such that it does not result in matching.
 */
TEST_F(MarketTest, DeleteOrderShouldWork1)
{
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

    // Delete the order.
    market.deleteOrder(symbol_id, id1);

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that first order was deleted - order should be unchanged.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order, order1);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests deleting an order such that it results in matching.
 */
TEST_F(MarketTest, DeleteOrderShouldWork2)
{
    // Order to add.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderType type1 = OrderType::AllOrNone;
    uint32_t quantity1 = 101;
    uint32_t price1 = 100;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Ask;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 100;
    uint32_t price2 = 105;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderAction action3 = OrderAction::Limit;
    OrderSide side3 = OrderSide::Bid;
    OrderType type3 = OrderType::GoodTillCancel;
    uint32_t quantity3 = 100;
    uint32_t price3 = 150;
    uint64_t id3 = 3;
    Order order3{action3, side3, type3, symbol_id, price3, quantity3, id3};

    // Add the order.
    market.addOrder(order3);

    // Delete the order.
    market.deleteOrder(symbol_id, id1);

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

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that the second order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), 0);

    // Check that the first order was deleted - order should be unchanged.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order, order1);

    // Check that third order was deleted since it was filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    // Check that second order was deleted since it was filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification3 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}
