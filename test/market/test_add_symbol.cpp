#include <gtest/gtest.h>
#include "market.h"
#include "debug_event_handler.h"

TEST(MarketTestSymbol, AddSymbolShouldWork1)
{
    DebugEventHandler event_handler;
    RapidTrader::Matching::Market market(event_handler.getSender());

    // Symbol data.
    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";

    // Add the symbol.
    market.addSymbol(symbol_id, symbol_name);

    event_handler.stop();

    // Check that symbol was added.
    ASSERT_TRUE(market.hasSymbol(symbol_id));
    ASSERT_FALSE(event_handler.add_symbol_notifications.empty());
    ASSERT_EQ(event_handler.add_symbol_notifications.front().symbol_id, symbol_id);
    ASSERT_EQ(event_handler.add_symbol_notifications.front().name, symbol_name);
    event_handler.add_symbol_notifications.pop();
    ASSERT_TRUE(event_handler.empty());
}

TEST(MarketTestSymbol, AddSymbolShouldWork2)
{
    DebugEventHandler event_handler;
    RapidTrader::Matching::Market market(event_handler.getSender());

    // Symbol data.
    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";

    // Add the symbol.
    market.addSymbol(symbol_id, symbol_name);

    // Try to add duplicate symbol
    ASSERT_EQ(market.addSymbol(symbol_id, symbol_name), ErrorStatus::DuplicateSymbol);

    event_handler.stop();
}
