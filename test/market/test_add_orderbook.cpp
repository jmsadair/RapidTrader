#include <gtest/gtest.h>
#include "market.h"
#include "debug_event_handler.h"

TEST(MarketTestOrderBook, AddOrderBookShouldWork1)
{
    DebugEventHandler event_handler;
    RapidTrader::Matching::Market market(event_handler.getSender());

    // Symbol data.
    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";

    // Add the symbol.
    market.addSymbol(symbol_id, symbol_name);

    // Add the orderbook for the symbol.
    market.addOrderbook(symbol_id);

    event_handler.stop();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(event_handler.add_symbol_notifications.empty());
    ASSERT_EQ(event_handler.add_symbol_notifications.front().symbol_id, symbol_id);
    ASSERT_EQ(event_handler.add_symbol_notifications.front().name, symbol_name);
    event_handler.add_symbol_notifications.pop();

    // Check that book was added.
    ASSERT_TRUE(market.hasOrderbook(symbol_id));
    ASSERT_FALSE(event_handler.add_book_notifications.empty());
    ASSERT_EQ(event_handler.add_book_notifications.front().symbol_id, symbol_id);
    event_handler.add_book_notifications.pop();
    ASSERT_TRUE(event_handler.empty());
}

TEST(MarketTestOrderBook, AddOrderBookShouldWork2)
{
    DebugEventHandler event_handler;

    RapidTrader::Matching::Market market(event_handler.getSender());

    // Symbol data.
    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";

    // Add the symbol.
    market.addSymbol(symbol_id, symbol_name);

    // Add the orderbook for the symbol.
    market.addOrderbook(symbol_id);

    // Try to add duplicate orderbook
    ASSERT_EQ(market.addOrderbook(symbol_id), ErrorStatus::DuplicateOrderBook);

    event_handler.stop();
}