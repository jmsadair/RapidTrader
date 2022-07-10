#include "market_test_fixture.h"

/**
 * Tests cancelling a portion of an order's quantity such that it is not removed from the book.
 */
TEST_F(MarketTest, CancelOrderShouldWork1)
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

    // Quantity to cancel.
    uint64_t cancel_quantity = 100;

    // Execute the order.
    market.cancelOrder(symbol_id, id1, cancel_quantity);

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that first order was cancelled.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification1 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity1 - cancel_quantity);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity1 - cancel_quantity);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests cancelling a portion of an order's quantity such that it is removed from the book.
 */
TEST_F(MarketTest, CancelOrderShouldWork2)
{
    // Order to add.
    OrderAction action1 = OrderAction::Stop;
    OrderSide side1 = OrderSide::Ask;
    OrderType type1 = OrderType::ImmediateOrCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Quantity to cancel.
    uint64_t cancel_quantity = 200;

    // Execute the order.
    market.cancelOrder(symbol_id, id1, cancel_quantity);

    notification_processor.shutdown();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification1 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that first order was cancelled.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification1 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), cancel_quantity);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), 0);

    // Check that the first order was deleted.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(delete_order_notification1.order.getQuantity(), cancel_quantity);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}
