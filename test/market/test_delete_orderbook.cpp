#include <gtest/gtest.h>
#include "market.h"
#include "debug_event_handler.h"

TEST(MarketTestOrderBook, DeleteOrderBookShouldWork1)
{
    DebugEventHandler event_handler;
    RapidTrader::Matching::Market market(event_handler.getSender());

    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";
    market.addSymbol(symbol_id, symbol_name);
    ASSERT_TRUE(market.hasSymbol(symbol_id));

    market.addOrderbook(symbol_id);
    ASSERT_TRUE(market.hasOrderbook(symbol_id));

    market.deleteOrderbook(symbol_id);
    ASSERT_FALSE(market.hasOrderbook(symbol_id));

    event_handler.stop();

    ASSERT_FALSE(event_handler.add_symbol_events.empty());
    ASSERT_EQ(event_handler.add_symbol_events.front().symbol_id, symbol_id);
    ASSERT_EQ(event_handler.add_symbol_events.front().name, symbol_name);
    event_handler.add_symbol_events.pop();

    ASSERT_FALSE(event_handler.add_book_events.empty());
    ASSERT_EQ(event_handler.add_book_events.front().symbol_id, symbol_id);
    event_handler.add_book_events.pop();

    ASSERT_FALSE(event_handler.delete_book_events.empty());
    ASSERT_EQ(event_handler.delete_book_events.front().symbol_id, symbol_id);
    event_handler.delete_book_events.pop();

    ASSERT_TRUE(event_handler.empty());
}