#include "market.h"
#include "map_orderbook.h"

namespace RapidTrader::Matching {
Market::Market(Concurrent::Messaging::Sender outgoing_messages_)
    : outgoing_messages(outgoing_messages_)
    , orderbook_handler(std::make_unique<OrderBookHandler>(outgoing_messages_))
{}

void Market::addSymbol(uint32_t symbol_id, const std::string &symbol_name)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
    {
        outgoing_messages.send(SymbolAdded{symbol_id, symbol_name});
        id_to_symbol.insert({symbol_id, std::make_unique<Symbol>(symbol_id, symbol_name)});
        orderbook_handler->addOrderBook(symbol_id);
        return;
    }
}

bool Market::hasSymbol(uint32_t symbol_id) const
{
    return id_to_symbol.find(symbol_id) != id_to_symbol.end();
}

void Market::addOrder(const Order &order)
{
    orderbook_handler->addOrder(order);
}

void Market::deleteOrder(uint32_t symbol_id, uint64_t order_id)
{
    orderbook_handler->deleteOrder(symbol_id, order_id);
}

void Market::cancelOrder(uint32_t symbol_id, uint64_t order_id, uint64_t cancelled_quantity)
{
    orderbook_handler->cancelOrder(symbol_id, order_id, cancelled_quantity);
}

void Market::replaceOrder(uint32_t symbol_id, uint64_t order_id, uint64_t new_order_id, uint64_t new_price)
{
    orderbook_handler->replaceOrder(symbol_id, order_id, new_order_id, new_price);
}

void Market::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint64_t price)
{
    orderbook_handler->executeOrder(symbol_id, order_id, quantity, price);
}

void Market::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity)
{
    orderbook_handler->executeOrder(symbol_id, order_id, quantity);
}

// LCOV_EXCL_START
std::string Market::toString() const
{
    return orderbook_handler->toString();
}

std::ostream &operator<<(std::ostream &os, const Market &market)
{
    os << market.orderbook_handler->toString();
    return os;
}

void Market::dumpMarket(const std::string &path) const
{
    std::ofstream file(path);
    file << *this;
    file.close();
}
// LCOV_EXCL_END
} // namespace RapidTrader::Matching