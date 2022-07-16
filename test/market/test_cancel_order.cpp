#include "market_test_fixture.h"

/**
 * Tests cancelling a portion of an order's quantity such that it is not removed from the book.
 */
TEST_F(MarketTest, CancelOrderShouldWork1)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);

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
    OrderTimeInForce tof1 = OrderTimeInForce::IOC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::stopAskOrder(id1, symbol_id, price1, quantity1, tof1);

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

/**
 * Tests cancelling an order with invalid parameters returns an error.
 */
TEST_F(MarketTest, CancelOrderShouldWork3)
{
    // Order to add.
    OrderTimeInForce tof1 = OrderTimeInForce::IOC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::stopAskOrder(id1, symbol_id, price1, quantity1, tof1);

    // Add the order.
    market.addOrder(order1);

    // Invalid execution data.
    uint64_t invalid_cancel_quantity = 0;
    uint64_t invalid_cancel_id = 0;
    uint64_t invalid_symbol_id = 0;

    // Cancel the order with invalid quantity.
    ASSERT_EQ(market.cancelOrder(symbol_id, id1, invalid_cancel_quantity), ErrorStatus::InvalidQuantity);
    // Cancel the order with invalid order ID - order does not exist.
    ASSERT_EQ(market.cancelOrder(symbol_id, invalid_cancel_id, quantity1), ErrorStatus::OrderDoesNotExist);
    // Cancel the order with invalid symbol ID - symbol does not exist.
    ASSERT_EQ(market.cancelOrder(invalid_symbol_id, quantity1, quantity1), ErrorStatus::SymbolDoesNotExist);

    // Symbol data.
    uint32_t new_symbol_id = 2;
    std::string new_symbol_name = "MSFT";

    // Add the symbol but not the orderbook.
    market.addSymbol(new_symbol_id, new_symbol_name);

    // Cancel with invalid orderbook ID - orderbook does not exist.
    ASSERT_EQ(market.cancelOrder(new_symbol_id, id1, quantity1), ErrorStatus::OrderBookDoesNotExist);

    notification_processor.shutdown();

    // Check that first order was added.
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
