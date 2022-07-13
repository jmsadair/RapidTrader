#include "market_test_fixture.h"

/**
 * Tests deleting an order such that it does not result in matching.
 */
TEST_F(MarketTest, DeleteOrderShouldWork1)
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