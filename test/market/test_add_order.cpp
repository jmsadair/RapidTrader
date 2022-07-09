#include <gtest/gtest.h>
#include "market.h"
#include "debug_notification_processor.h"

/**
 * Tests adding GTC limit order to empty orderbook.
 */
TEST(MarketTest, AddGtcLimitOrder1)
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
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().symbol_id, symbol_id);
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().name, symbol_name);
    notification_processor.add_symbol_notifications.pop();

    // Check that book was added.
    ASSERT_TRUE(market.hasOrderbook(symbol_id));
    ASSERT_EQ(notification_processor.add_book_notifications.front().symbol_id, symbol_id);
    notification_processor.add_book_notifications.pop();

    // Check that order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &notification = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(notification.order, order1);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding GTC limit orders that are matchable to an orderbook.
 */
TEST(MarketTest, AddGtcLimitOrder2)
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
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Ask;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 500;
    uint32_t price2 = 200;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
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

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that first order was executed - should be completely filled.
    // Note that bid orders are always processed first.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity2 - quantity1);

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding AON limit order that is matched when it is added to the book.
 */
TEST(MarketTest, AddAonLimitOrder1)
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
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Ask;
    OrderType type2 = OrderType::AllOrNone;
    uint32_t quantity2 = 100;
    uint32_t price2 = 200;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
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

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity1 - quantity2);

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);

    // Check that the second order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding limit IOC order that is not able to be completely filled to an orderbook.
 */
TEST(MarketTest, AddIocLimitOrder1)
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

    // Order to add.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Ask;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 100;
    uint32_t price2 = 400;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderAction action3 = OrderAction::Limit;
    OrderSide side3 = OrderSide::Bid;
    OrderType type3 = OrderType::ImmediateOrCancel;
    uint32_t quantity3 = 300;
    uint32_t price3 = 450;
    uint64_t id3 = 3;
    Order order3{action3, side3, type3, symbol_id, price3, quantity3, id3};

    // Add the order.
    market.addOrder(order3);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
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

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity3 - quantity1);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), 0);

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that the second order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    // Check that the third was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification3 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding limit IOC order that is not able to be completely filled to an orderbook.
 */
TEST(MarketTest, AddIocLimitOrder2)
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
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Ask;
    OrderType type2 = OrderType::ImmediateOrCancel;
    uint32_t quantity2 = 300;
    uint32_t price2 = 300;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
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

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity2 - quantity1);

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that the second order was deleted since it could not be filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), quantity2 - quantity1);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding limit FOK order that is able to be completely filled to an orderbook.
 */
TEST(MarketTest, AddFokLimitOrder1)
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

    // Order to add.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Ask;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 100;
    uint32_t price2 = 400;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderAction action3 = OrderAction::Limit;
    OrderSide side3 = OrderSide::Bid;
    OrderType type3 = OrderType::FillOrKill;
    uint32_t quantity3 = 250;
    uint32_t price3 = 450;
    uint64_t id3 = 3;
    Order order3{action3, side3, type3, symbol_id, price3, quantity3, id3};

    // Add the order.
    market.addOrder(order3);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
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

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity3 - quantity1);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), quantity2 - (quantity3 - quantity1));

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that the third was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding limit FOK order that is not able to be completely filled to an orderbook.
 */
TEST(MarketTest, AddFokLimitOrder2)
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
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Bid;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 100;
    uint32_t price2 = 400;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderAction action3 = OrderAction::Limit;
    OrderSide side3 = OrderSide::Ask;
    OrderType type3 = OrderType::FillOrKill;
    uint32_t quantity3 = 1000;
    uint32_t price3 = 450;
    uint64_t id3 = 3;
    Order order3{action3, side3, type3, symbol_id, price3, quantity3, id3};

    // Add the order.
    market.addOrder(order3);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
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

    // Check that the third was deleted since it could not be filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order, order3);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding market IOC order that is not able to be completely filled to an orderbook.
 */
TEST(MarketTest, AddIocMarketOrder1)
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
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Bid;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 100;
    uint32_t price2 = 250;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderAction action3 = OrderAction::Market;
    OrderSide side3 = OrderSide::Ask;
    OrderType type3 = OrderType::ImmediateOrCancel;
    uint32_t quantity3 = 500;
    uint32_t price3 = 0;
    uint64_t id3 = 3;
    Order order3{action3, side3, type3, symbol_id, price3, quantity3, id3};

    // Add the order.
    market.addOrder(order3);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
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

    // Check that first order was executed - should be completely filled.
    // Note that bid orders are always processed first.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity3 - quantity1);

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), quantity3 - quantity2 - quantity1);

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that the second order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    // Check that third order was deleted since it could not be filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification3 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), quantity3 - quantity2 - quantity1);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding market IOC order that is able to be completely filled to an orderbook.
 */
TEST(MarketTest, AddIocMarketOrder2)
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

    // Order to add.
    OrderAction action2 = OrderAction::Market;
    OrderSide side2 = OrderSide::Bid;
    OrderType type2 = OrderType::ImmediateOrCancel;
    uint32_t quantity2 = 100;
    uint32_t price2 = 0;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
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

    // Check that second order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification2 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification2.order, order2);

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity1 - quantity2);

    // Check that the second order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity2);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding stop IOC order that is activated when it is added to the book.
 */
TEST(MarketTest, AddIocStopOrder1)
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
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 450;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Bid;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 100;
    uint32_t price2 = 250;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderAction action3 = OrderAction::Stop;
    OrderSide side3 = OrderSide::Ask;
    OrderType type3 = OrderType::ImmediateOrCancel;
    uint32_t quantity3 = 500;
    uint32_t price3 = 249;
    uint64_t id3 = 3;
    Order order3{action3, side3, type3, symbol_id, price3, quantity3, id3};

    // Add the order.
    market.addOrder(order3);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
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

    // Check that third order was updated to be a market order since it was activated.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification1 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(update_order_notification1.order.getAction(), OrderAction::Market);
    ASSERT_EQ(update_order_notification1.order.getType(), type3);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity3);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity3);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity3 - quantity1);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), quantity2 - (quantity3 - quantity1));

    // Check that third order was executed - should be fully filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), 0);

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that third order was deleted since it was IOC.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification3 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding stop IOC order that is activated after new limit order is added to the book.
 */
TEST(MarketTest, AddIocStopOrder2)
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
    uint32_t quantity1 = 100;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderAction action2 = OrderAction::Stop;
    OrderSide side2 = OrderSide::Bid;
    OrderType type2 = OrderType::ImmediateOrCancel;
    uint32_t quantity2 = 100;
    uint32_t price2 = 349;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderAction action3 = OrderAction::Limit;
    OrderSide side3 = OrderSide::Ask;
    OrderType type3 = OrderType::GoodTillCancel;
    uint32_t quantity3 = 50;
    uint32_t price3 = 349;
    uint64_t id3 = 3;
    Order order3{action3, side3, type3, symbol_id, price3, quantity3, id3};

    // Add the order.
    market.addOrder(order3);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
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

    // Check that third order was updated to be a market order since it was activated.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification1 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(update_order_notification1.order.getAction(), OrderAction::Market);
    ASSERT_EQ(update_order_notification1.order.getType(), type2);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity2);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity2);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), quantity2 - quantity3);

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be fully filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity2 - quantity3);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), 0);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity2 - quantity3);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), quantity1 - (quantity2 - quantity3));

    // Check that the third order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that second order was deleted since it was IOC.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification3 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity2 - quantity3);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding multiple stop IOC orders that are activated after new limit order is added to the book.
 */
TEST(MarketTest, AddIocStopOrder3)
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
    uint32_t quantity1 = 500;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol_id, price1, quantity1, id1};

    // Add the order.
    market.addOrder(order1);

    // Order to add.
    OrderAction action2 = OrderAction::Stop;
    OrderSide side2 = OrderSide::Bid;
    OrderType type2 = OrderType::ImmediateOrCancel;
    uint32_t quantity2 = 100;
    uint32_t price2 = 320;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderAction action3 = OrderAction::Stop;
    OrderSide side3 = OrderSide::Bid;
    OrderType type3 = OrderType::ImmediateOrCancel;
    uint32_t quantity3 = 200;
    uint32_t price3 = 310;
    uint64_t id3 = 3;
    Order order3{action3, side3, type3, symbol_id, price3, quantity3, id3};

    // Add the order.
    market.addOrder(order3);

    // Order to add.
    OrderAction action4 = OrderAction::Stop;
    OrderSide side4 = OrderSide::Bid;
    OrderType type4 = OrderType::ImmediateOrCancel;
    uint32_t quantity4 = 220;
    uint32_t price4 = 305;
    uint64_t id4 = 4;
    Order order4{action4, side4, type4, symbol_id, price4, quantity4, id4};

    // Add the order.
    market.addOrder(order4);

    // Order to add.
    OrderAction action5 = OrderAction::Limit;
    OrderSide side5 = OrderSide::Ask;
    OrderType type5 = OrderType::GoodTillCancel;
    uint32_t quantity5 = 50;
    uint32_t price5 = 300;
    uint64_t id5 = 5;
    Order order5{action5, side5, type5, symbol_id, price5, quantity5, id5};

    // Add the order.
    market.addOrder(order5);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
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

    // Check that fourth order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification4 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification4.order, order4);

    // Check that fifth order was added. Order should be identical to original
    // order since it should not have been matched.
    ASSERT_FALSE(notification_processor.add_order_notifications.empty());
    AddedOrder &add_order_notification5 = notification_processor.add_order_notifications.front();
    notification_processor.add_order_notifications.pop();
    ASSERT_EQ(add_order_notification5.order, order5);

    // Check that second order was updated to be a market order since it was activated.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification1 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(update_order_notification1.order.getAction(), OrderAction::Market);
    ASSERT_EQ(update_order_notification1.order.getType(), type2);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity2);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity2);

    // Check that third order was updated to be a market order since it was activated.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification2 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification2.order.getOrderID(), id3);
    ASSERT_EQ(update_order_notification2.order.getAction(), OrderAction::Market);
    ASSERT_EQ(update_order_notification2.order.getType(), type3);
    ASSERT_EQ(update_order_notification2.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification2.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification2.order.getQuantity(), quantity3);
    ASSERT_EQ(update_order_notification2.order.getOpenQuantity(), quantity3);

    // Check that fourth order was updated to be a market order since it was activated.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification3 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification3.order.getOrderID(), id4);
    ASSERT_EQ(update_order_notification3.order.getAction(), OrderAction::Market);
    ASSERT_EQ(update_order_notification3.order.getType(), type4);
    ASSERT_EQ(update_order_notification3.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification3.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification3.order.getQuantity(), quantity4);
    ASSERT_EQ(update_order_notification3.order.getOpenQuantity(), quantity4);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity2 - quantity5);

    // Check that fifth order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id5);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity2 - quantity5);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity2 - quantity5);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), quantity1 - (quantity2 - quantity5));

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification5 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification5.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification5.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification5.order.getOpenQuantity(), 0);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification6 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification6.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification6.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(execute_order_notification6.order.getOpenQuantity(), quantity1 - (quantity2 - quantity5) - quantity3);

    // Check that fourth order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification7 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification7.order.getOrderID(), id4);
    ASSERT_EQ(execute_order_notification7.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification7.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(execute_order_notification7.order.getOpenQuantity(), 0);

    // Check that first order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification8 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification8.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification8.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification8.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(execute_order_notification8.order.getOpenQuantity(), quantity1 - (quantity2 - quantity5) - quantity3 - quantity4);

    // Check that fifth order was deleted since it was filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id5);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price5);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity5);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that second order was deleted since it was IOC.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification2 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification2.order.getOrderID(), id2);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification2.order.getLastExecutedQuantity(), quantity2 - quantity5);
    ASSERT_EQ(delete_order_notification2.order.getOpenQuantity(), 0);

    // Check that third order was deleted since it was IOC.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification3 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity3);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);

    // Check that the fourth order was deleted since it was IOC.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification4 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification4.order.getOrderID(), id4);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(delete_order_notification4.order.getLastExecutedQuantity(), quantity4);
    ASSERT_EQ(delete_order_notification4.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding stop limit GTC order that is activated when it is added to the book.
 */
TEST(MarketTest, AddGtcStopLimitOrder1)
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

    // Order to add.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Ask;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 350;
    uint32_t price2 = 400;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol_id, price2, quantity2, id2};

    // Add the order.
    market.addOrder(order2);

    // Order to add.
    OrderAction action3 = OrderAction::StopLimit;
    OrderSide side3 = OrderSide::Bid;
    OrderType type3 = OrderType::GoodTillCancel;
    uint32_t quantity3 = 500;
    uint32_t price3 = 500;
    uint64_t id3 = 3;
    Order order3{action3, side3, type3, symbol_id, price3, quantity3, id3};

    // Add the order.
    market.addOrder(order3);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
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

    // Check that third order was updated to be a limit order since it was activated.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification1 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(update_order_notification1.order.getAction(), OrderAction::Limit);
    ASSERT_EQ(update_order_notification1.order.getType(), type3);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity3);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity3);

    // Check that third order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification1 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification1.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedPrice(), price1);
    ASSERT_EQ(execute_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification1.order.getOpenQuantity(), quantity3 - quantity1);

    // Check that first order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification2 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification2.order.getOrderID(), id1);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification2.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(execute_order_notification2.order.getOpenQuantity(), 0);

    // Check that third order was executed - should be completely filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification3 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(execute_order_notification3.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification3.order.getOpenQuantity(), 0);

    // Check that second order was executed - should be partially filled.
    ASSERT_FALSE(notification_processor.execute_order_notifications.empty());
    ExecutedOrder &execute_order_notification4 = notification_processor.execute_order_notifications.front();
    notification_processor.execute_order_notifications.pop();
    ASSERT_EQ(execute_order_notification4.order.getOrderID(), id2);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(execute_order_notification4.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(execute_order_notification4.order.getOpenQuantity(), quantity2 - (quantity3 - quantity1));

    // Check that the first order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedPrice(), price3);
    ASSERT_EQ(delete_order_notification1.order.getLastExecutedQuantity(), quantity1);
    ASSERT_EQ(delete_order_notification1.order.getOpenQuantity(), 0);

    // Check that third order was deleted since it was completely filled.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification3 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification3.order.getOrderID(), id3);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedPrice(), price2);
    ASSERT_EQ(delete_order_notification3.order.getLastExecutedQuantity(), quantity3 - quantity1);
    ASSERT_EQ(delete_order_notification3.order.getOpenQuantity(), 0);

    ASSERT_TRUE(notification_processor.empty());
}