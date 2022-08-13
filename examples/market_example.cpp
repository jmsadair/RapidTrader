#include <iostream>
#include "market/market.h"
#include "event_handler.h"

class SimpleEventHandler : public EventHandler
{
protected:
    void handleOrderAdded(const OrderAdded &notification) override
    {
        std::cout << notification << "\n";
    }
    void handleOrderDeleted(const OrderDeleted &notification) override
    {
        std::cout << notification << "\n";
    }
    void handleOrderUpdated(const OrderUpdated &notification) override
    {
        std::cout << notification << "\n";
    }
    void handleOrderExecuted(const ExecutedOrder &notification) override
    {
        std::cout << notification << "\n";
    }
    void handleSymbolAdded(const SymbolAdded &notification) override
    {
        std::cout << notification << "\n";
    }
    void handleSymbolDeleted(const SymbolDeleted &notification) override
    {
        std::cout << notification << "\n";
    }
};

int main()
{
    // Create a new market with an event handler. Note that the market requires ownership of the event handler.
    auto event_handler = std::unique_ptr<EventHandler>(new SimpleEventHandler);
    RapidTrader::Matching::Market market{std::move(event_handler)};

    // Add a new symbol to the market. A symbol must be added to the market before any orders with that symbol are submitted.
    std::string symbol_name = "GOOG";
    uint32_t symbol_id = 1;
    market.addSymbol(symbol_id, symbol_name);

    // Add some limit orders to the market.
    OrderTimeInForce time_in_force1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, time_in_force1);
    market.addOrder(order1);

    OrderTimeInForce time_in_force2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 200;
    uint64_t price2 = 350;
    uint64_t id2 = 2;
    Order order2 = Order::limitAskOrder(id2, symbol_id, price2, quantity2, time_in_force2);
    market.addOrder(order2);

    return 0;
}
