#ifndef RAPID_TRADER_MARKET_TEST_FIXTURE_H
#define RAPID_TRADER_MARKET_TEST_FIXTURE_H
#include <gtest/gtest.h>
#include "debug_notification_processor.h"
#include "market.h"

class MarketTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        notification_processor.run();
        // Add the symbol.
        market.addSymbol(symbol_id, symbol_name);
        // Add the orderbook for the symbol.
        market.addOrderbook(symbol_id);
        // Make sure the market has the symbol and orderbook.
        notification_processor.shutdown();
        assert(market.hasSymbol(symbol_id) && "Symbol was not added to market!");
        assert(market.hasOrderbook(symbol_id) && "Orderbook was not added to market!");
        assert(!notification_processor.add_symbol_notifications.empty() && "Never received notification that symbol was added!");
        assert(!notification_processor.add_book_notifications.empty() && "Never received notification that orderbook was added!");
        notification_processor.add_symbol_notifications.pop();
        notification_processor.add_book_notifications.pop();
        assert(notification_processor.empty() && "There should not be any notifications!");
        notification_processor.run();
    }

    DebugNotificationProcessor notification_processor;
    RapidTrader::Matching::Market market{notification_processor.getSender()};
    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";
};
#endif // RAPID_TRADER_MARKET_TEST_FIXTURE_H
