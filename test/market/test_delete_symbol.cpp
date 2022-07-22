#include <gtest/gtest.h>
#include "market.h"
#include "debug_event_handler.h"

TEST(MarketTestSymbol, DeleteSymbolShouldWork1)
{
    DebugEventHandler event_handler;
    RapidTrader::Matching::Market market(event_handler.getSender());

    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";
    market.addSymbol(symbol_id, symbol_name);
    ASSERT_TRUE(market.hasSymbol(symbol_id));

    market.deleteSymbol(symbol_id);
    ASSERT_FALSE(market.hasSymbol(symbol_id));

    event_handler.stop();

    ASSERT_FALSE(event_handler.add_symbol_events.empty());
    ASSERT_EQ(event_handler.add_symbol_events.front().symbol_id, symbol_id);
    ASSERT_EQ(event_handler.add_symbol_events.front().name, symbol_name);
    event_handler.add_symbol_events.pop();

    ASSERT_FALSE(event_handler.delete_symbol_events.empty());
    ASSERT_EQ(event_handler.delete_symbol_events.front().symbol_id, symbol_id);
    ASSERT_EQ(event_handler.delete_symbol_events.front().name, symbol_name);
    event_handler.delete_symbol_events.pop();

    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests deleting a symbol that does not exist returns an error.
 */
TEST(MarketTestSymbol, DeleteSymbolShouldWork2)
{
    DebugEventHandler event_handler;
    RapidTrader::Matching::Market market(event_handler.getSender());

    uint32_t invalid_symbol_id = 0;
    ASSERT_EQ(market.deleteSymbol(invalid_symbol_id), ErrorStatus::SymbolDoesNotExist);

    event_handler.stop();

    ASSERT_TRUE(event_handler.empty());
}
