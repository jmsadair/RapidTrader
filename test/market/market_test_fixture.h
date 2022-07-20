#ifndef RAPID_TRADER_MARKET_TEST_FIXTURE_H
#define RAPID_TRADER_MARKET_TEST_FIXTURE_H
#include <gtest/gtest.h>
#include "debug_event_handler.h"
#include "market.h"

class MarketTest : public ::testing::Test
{
protected:
    MarketTest()
        : market(event_handler.getSender())
    {}

    void SetUp() override
    {
        // Add the symbol.
        market.addSymbol(symbol_id, symbol_name);
        // Add the orderbook for the symbol.
        market.addOrderbook(symbol_id);
        // Make sure the market has the symbol and orderbook.
        assert(market.hasSymbol(symbol_id) && "Symbol was not added to market!");
        assert(market.hasOrderbook(symbol_id) && "Orderbook was not added to market!");
        // Join the event handling thread and make sure that add symbol and add orderbook events were received.
        event_handler.stop();
        assert(!event_handler.add_symbol_notifications.empty() && "There should have been a symbol added event!");
        assert(!event_handler.add_book_notifications.empty() && "There should have been a orderbook added event!");
        event_handler.add_symbol_notifications.pop();
        event_handler.add_book_notifications.pop();
        assert(event_handler.empty() && "There should not be any notifications!");
        // Restart the event handling thread.
        event_handler.start();
    }

    DebugEventHandler event_handler;
    RapidTrader::Matching::Market market;
    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";
};
#endif // RAPID_TRADER_MARKET_TEST_FIXTURE_H
