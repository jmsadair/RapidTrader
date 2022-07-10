#include <gtest/gtest.h>
#include "market.h"
#include "debug_notification_processor.h"

TEST(MarketTestOrderBook, DeleteOrderBookShouldWork1)
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