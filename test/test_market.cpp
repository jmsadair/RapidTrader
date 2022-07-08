#include <gtest/gtest.h>
#include <vector>
#include <thread>
#include "market.h"
#include "notification_processor.h"

class DebugNotificationProcessor : public NotificationProcessor
{
public:
    std::queue<AddedSymbol> add_symbol_notifications;
    std::queue<DeletedSymbol> delete_symbol_notifications;
    std::queue<AddedOrderBook> add_book_notifications;
    std::queue<DeletedOrderBook> delete_book_notifications;
    std::queue<AddedOrder> add_order_notifications;
    std::queue<DeletedOrder> delete_order_notifications;
    std::queue<ExecutedOrder> execute_order_notifications;
    std::queue<UpdatedOrder> update_order_notifications;

    [[nodiscard]] bool empty() const
    {
        return add_symbol_notifications.empty() && delete_symbol_notifications.empty() && add_book_notifications.empty() &&
               delete_book_notifications.empty() && add_order_notifications.empty() && delete_order_notifications.empty() &&
               execute_order_notifications.empty() && update_order_notifications.empty();
    }

protected:
    void onOrderAdded(const AddedOrder &notification) override
    {
        add_order_notifications.push(notification);
    }
    void onOrderDeleted(const DeletedOrder &notification) override
    {
        delete_order_notifications.push(notification);
    }
    void onOrderUpdated(const UpdatedOrder &notification) override
    {
        update_order_notifications.push(notification);
    }
    void onOrderExecuted(const ExecutedOrder &notification) override
    {
        execute_order_notifications.push(notification);
    }
    void onSymbolAdded(const AddedSymbol &notification) override
    {
        add_symbol_notifications.push(notification);
    }
    void onSymbolDeleted(const DeletedSymbol &notification) override
    {
        delete_symbol_notifications.push(notification);
    }
    void onOrderBookAdded(const AddedOrderBook &notification) override
    {
        add_book_notifications.push(notification);
    }
    void onOrderBookDeleted(const DeletedOrderBook &notification) override
    {
        delete_book_notifications.push(notification);
    }
};

TEST(MarketTest, AddSymbolShouldWork1)
{
    DebugNotificationProcessor notification_processor;
    notification_processor.run();
    RapidTrader::Matching::Market market(notification_processor.getSender());

    // Symbol data.
    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";

    // Add the symbol.
    market.addSymbol(symbol_id, symbol_name);

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().symbol_id, symbol_id);
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().name, symbol_name);
    notification_processor.add_symbol_notifications.pop();
    ASSERT_TRUE(notification_processor.empty());
}

TEST(MarketTest, DeleteSymbolShouldWork1)
{
    DebugNotificationProcessor notification_processor;
    notification_processor.run();
    RapidTrader::Matching::Market market(notification_processor.getSender());

    // Symbol data.
    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";

    // Add the symbol.
    market.addSymbol(symbol_id, symbol_name);
    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));

    // Delete the symbol.
    market.deleteSymbol(symbol_id);
    // Check that the symbol was deleted.
    ASSERT_FALSE(market.hasSymbol(symbol_id));

    notification_processor.shutdown();

    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().symbol_id, symbol_id);
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().name, symbol_name);
    notification_processor.add_symbol_notifications.pop();

    ASSERT_FALSE(notification_processor.delete_symbol_notifications.empty());
    ASSERT_EQ(notification_processor.delete_symbol_notifications.front().symbol_id, symbol_id);
    ASSERT_EQ(notification_processor.delete_symbol_notifications.front().name, symbol_name);
    notification_processor.delete_symbol_notifications.pop();

    ASSERT_TRUE(notification_processor.empty());
}

TEST(MarketTest, AddOrderBookShouldWork1)
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

    notification_processor.shutdown();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().symbol_id, symbol_id);
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().name, symbol_name);
    notification_processor.add_symbol_notifications.pop();

    // Check that book was added.
    ASSERT_TRUE(market.hasOrderbook(symbol_id));
    ASSERT_FALSE(notification_processor.add_book_notifications.empty());
    ASSERT_EQ(notification_processor.add_book_notifications.front().symbol_id, symbol_id);
    notification_processor.add_book_notifications.pop();
    ASSERT_TRUE(notification_processor.empty());
}

TEST(MarketTest, DeleteOrderBookShouldWork1)
{
    DebugNotificationProcessor notification_processor;
    notification_processor.run();
    RapidTrader::Matching::Market market(notification_processor.getSender());

    // Symbol data.
    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";

    // Add the symbol.
    market.addSymbol(symbol_id, symbol_name);

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));

    // Add the orderbook for the symbol.
    market.addOrderbook(symbol_id);

    // Check that book was added.
    ASSERT_TRUE(market.hasOrderbook(symbol_id));

    // Delete the orderbook for the symbol.
    market.deleteOrderbook(symbol_id);

    // Check that the orderbook was deleted.
    ASSERT_FALSE(market.hasOrderbook(symbol_id));

    notification_processor.shutdown();

    ASSERT_FALSE(notification_processor.add_symbol_notifications.empty());
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().symbol_id, symbol_id);
    ASSERT_EQ(notification_processor.add_symbol_notifications.front().name, symbol_name);
    notification_processor.add_symbol_notifications.pop();

    ASSERT_FALSE(notification_processor.add_book_notifications.empty());
    ASSERT_EQ(notification_processor.add_book_notifications.front().symbol_id, symbol_id);
    notification_processor.add_book_notifications.pop();

    ASSERT_FALSE(notification_processor.delete_book_notifications.empty());
    ASSERT_EQ(notification_processor.delete_book_notifications.front().symbol_id, symbol_id);
    notification_processor.delete_book_notifications.pop();

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests adding GTC limit order to empty orderbook.
 */
TEST(MarketTest, AddOrderShouldWork1)
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
TEST(MarketTest, AddOrderShouldWork2)
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
 * Tests adding limit IOC order that is not able to be completely filled to an orderbook.
 */
TEST(MarketTest, AddOrderShouldWork3)
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
 * Tests adding limit FOK order that is able to be completely filled to an orderbook.
 */
TEST(MarketTest, AddOrderShouldWork4)
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
 * Tests adding limit IOC order that is not able to be completely filled to an orderbook.
 */
TEST(MarketTest, AddOrderShouldWork5)
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
 * Tests adding limit FOK order that is not able to be completely filled to an orderbook.
 */
TEST(MarketTest, AddOrderShouldWork6)
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
TEST(MarketTest, AddOrderShouldWork7)
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
TEST(MarketTest, AddOrderShouldWork8)
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
 * Tests deleting an order.
 */
TEST(MarketTest, DeleteOrderShouldWork1)
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

    // Delete the order.
    market.deleteOrder(symbol_id, id1);

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

    // Check that first order was deleted - order should be unchanged.
    ASSERT_FALSE(notification_processor.delete_order_notifications.empty());
    DeletedOrder &delete_order_notification1 = notification_processor.delete_order_notifications.front();
    notification_processor.delete_order_notifications.pop();
    ASSERT_EQ(delete_order_notification1.order, order1);

    ASSERT_TRUE(notification_processor.empty());
}

/**
 * Tests replacing an order such that it does not result in matching.
 */
TEST(MarketTest, ReplaceOrderShouldWork1)
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
TEST(MarketTest, ReplaceOrderShouldWork2)
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

/**
 * Tests cancelling a portion of an orders quantity such that it is not removed from the book.
 */
TEST(MarketTest, CancelOrderShouldWork1)
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

    // Quantity to cancel.
    uint64_t cancel_quantity = 100;

    // Execute the order.
    market.cancelOrder(symbol_id, id1, cancel_quantity);

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

    // Check that first order was cancelled.
    ASSERT_FALSE(notification_processor.update_order_notifications.empty());
    UpdatedOrder &update_order_notification1 = notification_processor.update_order_notifications.front();
    notification_processor.update_order_notifications.pop();
    ASSERT_EQ(update_order_notification1.order.getOrderID(), id1);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedPrice(), 0);
    ASSERT_EQ(update_order_notification1.order.getLastExecutedQuantity(), 0);
    ASSERT_EQ(update_order_notification1.order.getQuantity(), quantity1);
    ASSERT_EQ(update_order_notification1.order.getOpenQuantity(), quantity1 - cancel_quantity);

    ASSERT_TRUE(notification_processor.empty());
}
