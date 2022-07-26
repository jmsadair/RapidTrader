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
        assert(!event_handler.add_symbol_events.empty() && "There should have been a symbol added event!");
        assert(!event_handler.add_book_events.empty() && "There should have been a orderbook added event!");
        event_handler.add_symbol_events.pop();
        event_handler.add_book_events.pop();
        assert(event_handler.empty() && "There should not be any notifications!");
        // Restart the event handling thread.
        event_handler.start();
    }

    static void checkOrder(const Order &order, uint64_t expected_order_id, uint64_t expected_last_execution_price,
        uint64_t expected_last_execution_quantity, uint64_t expected_open_quantity)
    {
        ASSERT_EQ(order.getOrderID(), expected_order_id);
        ASSERT_EQ(order.getLastExecutedPrice(), expected_last_execution_price);
        ASSERT_EQ(order.getLastExecutedQuantity(), expected_last_execution_quantity);
        ASSERT_EQ(order.getOpenQuantity(), expected_open_quantity);
    }

    void checkOrderAdded(uint64_t expected_order_id)
    {
        ASSERT_FALSE(event_handler.add_order_events.empty());
        Order &order_added = event_handler.add_order_events.front().order;
        ASSERT_EQ(order_added.getOrderID(), expected_order_id);
        event_handler.add_order_events.pop();
    }

    void checkExecutedOrder(uint64_t expected_order_id, uint64_t expected_last_execution_price, uint64_t expected_last_execution_quantity,
        uint64_t expected_open_quantity)
    {
        ASSERT_FALSE(event_handler.execute_order_events.empty());
        Order &order_executed = event_handler.execute_order_events.front().order;
        checkOrder(
            order_executed, expected_order_id, expected_last_execution_price, expected_last_execution_quantity, expected_open_quantity);
        event_handler.execute_order_events.pop();
    }

    void checkOrderDeleted(uint64_t expected_order_id, uint64_t expected_last_execution_price, uint64_t expected_last_execution_quantity,
        uint64_t expected_open_quantity)
    {
        ASSERT_FALSE(event_handler.delete_order_events.empty());
        Order &order_deleted = event_handler.delete_order_events.front().order;
        const OrderBook &book = market.getOrderbook(order_deleted.getSymbolID());
        ASSERT_FALSE(book.hasOrder(order_deleted.getOrderID()));
        checkOrder(
            order_deleted, expected_order_id, expected_last_execution_price, expected_last_execution_quantity, expected_open_quantity);
        event_handler.delete_order_events.pop();
    }

    void checkOrderUpdated(uint64_t expected_order_id, uint64_t expected_last_execution_price, uint64_t expected_last_execution_quantity,
        uint64_t expected_open_quantity)
    {
        ASSERT_FALSE(event_handler.update_order_events.empty());
        Order &order_updated = event_handler.update_order_events.front().order;
        checkOrder(
            order_updated, expected_order_id, expected_last_execution_price, expected_last_execution_quantity, expected_open_quantity);
        event_handler.update_order_events.pop();
    }

    void checkSymbolAdded(uint64_t expected_symbol_id, const std::string &expected_symbol_name)
    {
        ASSERT_FALSE(event_handler.add_symbol_events.empty());
        SymbolAdded &symbol_added_event = event_handler.add_symbol_events.front();
        ASSERT_EQ(symbol_added_event.symbol_id, expected_symbol_id);
        ASSERT_EQ(symbol_added_event.name, expected_symbol_name);
        event_handler.add_symbol_events.pop();
    }

    void checkSymbolDeleted(uint64_t expected_symbol_id, const std::string &expected_symbol_name)
    {
        ASSERT_FALSE(event_handler.delete_symbol_events.empty());
        SymbolDeleted &symbol_deleted_event = event_handler.delete_symbol_events.front();
        ASSERT_EQ(symbol_deleted_event.symbol_id, expected_symbol_id);
        ASSERT_EQ(symbol_deleted_event.name, expected_symbol_name);
        event_handler.delete_symbol_events.pop();
    }

    void checkBookAdded(uint64_t expected_symbol_id)
    {
        ASSERT_FALSE(event_handler.add_book_events.empty());
        OrderBookAdded &book_added_event = event_handler.add_book_events.front();
        ASSERT_EQ(book_added_event.symbol_id, expected_symbol_id);
        event_handler.add_book_events.pop();
    }

    void checkBookDeleted(uint64_t expected_symbol_id)
    {
        ASSERT_FALSE(event_handler.delete_book_events.empty());
        OrderBookDeleted &book_deleted_event = event_handler.delete_book_events.front();
        ASSERT_EQ(book_deleted_event.symbol_id, expected_symbol_id);
        event_handler.delete_book_events.pop();
    }

    DebugEventHandler event_handler;
    RapidTrader::Matching::Market market;
    uint32_t symbol_id = 1;
    std::string symbol_name = "GOOG";
};
#endif // RAPID_TRADER_MARKET_TEST_FIXTURE_H
