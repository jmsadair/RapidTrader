#include "market.h"
#include "map_orderbook.h"
#include "event_handler/event.h"

namespace RapidTrader::Matching {
Market::Market(Concurrent::Messaging::Sender outgoing_messages_)
    : outgoing_messages(outgoing_messages_)
{}

void Market::addSymbol(uint32_t symbol_id, const std::string &symbol_name)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
    {
        outgoing_messages.send(SymbolAdded{symbol_id, symbol_name});
        id_to_symbol.insert({symbol_id, std::make_unique<Symbol>(symbol_id, symbol_name)});
        id_to_book.insert({symbol_id, std::make_unique<MapOrderBook>(symbol_id, outgoing_messages)});
        return;
    }
}

bool Market::hasSymbol(uint32_t symbol_id) const
{
    return id_to_symbol.find(symbol_id) != id_to_symbol.end();
}

const OrderBook &Market::getOrderbook(uint32_t symbol_id) const
{
    return *id_to_book.find(symbol_id)->second;
}

void Market::addOrder(const Order &order)
{
    if (id_to_symbol.find(order.getSymbolID()) != id_to_symbol.end())
    {
        // Get orderbook that corresponds to order symbol ID.
        OrderBook *book = id_to_book.find(order.getSymbolID())->second.get();
        // Submit the order.
        book->addOrder(order);
    }
}

void Market::deleteOrder(uint32_t symbol_id, uint64_t order_id)
{
    if (id_to_symbol.find(symbol_id) != id_to_symbol.end())
    {
        // Get orderbook that corresponds to order symbol ID.
        OrderBook *book = id_to_book.find(symbol_id)->second.get();
        // Delete the order.
        book->deleteOrder(order_id);
    }
}

void Market::cancelOrder(uint32_t symbol_id, uint64_t order_id, uint64_t cancelled_quantity)
{
    if (id_to_symbol.find(symbol_id) != id_to_symbol.end())
    {
        // Get orderbook that corresponds to order symbol ID.
        OrderBook *book = id_to_book.find(symbol_id)->second.get();
        // Cancel the order.
        book->cancelOrder(order_id, cancelled_quantity);
    }
}
void Market::replaceOrder(uint32_t symbol_id, uint64_t order_id, uint64_t new_order_id, uint64_t new_price)
{
    if (id_to_symbol.find(symbol_id) != id_to_symbol.end())
    {
        // Get orderbook that corresponds to order symbol ID.
        OrderBook *book = id_to_book.find(symbol_id)->second.get();
        // Replace the order.
        book->replaceOrder(order_id, new_order_id, new_price);
    }
}

void Market::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint64_t price)
{
    if (id_to_symbol.find(symbol_id) != id_to_symbol.end())
    {
        // Get orderbook that corresponds to order symbol ID.
        OrderBook *book = id_to_book.find(symbol_id)->second.get();
        // Execute the order.
        book->executeOrder(order_id, quantity, price);
    }
}
void Market::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity)
{
    if (id_to_symbol.find(symbol_id) != id_to_symbol.end())
    {
        // Get orderbook that corresponds to order symbol ID.
        OrderBook *book = id_to_book.find(symbol_id)->second.get();
        // Execute the order.
        book->executeOrder(order_id, quantity);
    }
}

// LCOV_EXCL_START
std::string Market::toString() const
{
    std::string market_string;
    for (const auto &[symbol_id, book_ptr] : id_to_book)
    {
        if (book_ptr)
            market_string += book_ptr->toString() + "\n";
    }
    return market_string;
}

std::ostream &operator<<(std::ostream &os, const Market &market)
{
    for (const auto &[symbol_id, book_ptr] : market.id_to_book)
    {
        if (book_ptr)
            os << book_ptr->toString() << "\n";
    }
    return os;
}

void Market::dumpMarket(const std::string &path) const
{
    std::ofstream file(path);
    file << *this;
    file.close();
}
bool Market::hasOrderbook(uint32_t symbol_id) const
{
    return id_to_book.find(symbol_id) != id_to_book.end();
}
// LCOV_EXCL_END
} // namespace RapidTrader::Matching