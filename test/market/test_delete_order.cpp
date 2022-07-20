#include "market_test_fixture.h"

/**
 * Tests deleting an order such that it does not result in matching.
 */
TEST_F(MarketTest, DeleteOrderShouldWork1)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Delete the order.
    market.deleteOrder(symbol_id, id1);

    event_handler.stop();

    // Check that first order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(event_handler.add_order_notifications.empty());
    OrderAdded &add_order_notification1 = event_handler.add_order_notifications.front();
    event_handler.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification1.order, order1);

    // Check that first order was deleted - order should be unchanged.
    ASSERT_FALSE(event_handler.delete_order_notifications.empty());
    OrderDeleted &delete_order_notification1 = event_handler.delete_order_notifications.front();
    event_handler.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order, order1);

    ASSERT_TRUE(event_handler.empty());
}
