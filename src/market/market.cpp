#include "market/market.h"
#include "map_orderbook.h"

OrderBookHandler::OrderBookHandler(Concurrent::Messaging::Sender outgoing_messages_)
    : outgoing_messages(outgoing_messages_)
{}

void OrderBookHandler::addOrderBook(uint32_t symbol_id)
{
    if (id_to_book.find(symbol_id) == id_to_book.end())
        id_to_book.insert({symbol_id, std::make_unique<MapOrderBook>(symbol_id, outgoing_messages)});
}

void OrderBookHandler::deleteOrderBook(uint32_t symbol_id)
{
    if (id_to_book.find(symbol_id) != id_to_book.end())
        id_to_book.erase(symbol_id);
}

void OrderBookHandler::addOrder(const Order &order)
{
    if (id_to_book.find(order.getSymbolID()) != id_to_book.end())
    {
        OrderBook *book = id_to_book.find(order.getSymbolID())->second.get();
        book->addOrder(order);
    }
}

void OrderBookHandler::deleteOrder(uint32_t symbol_id, uint64_t order_id)
{
    if (id_to_book.find(symbol_id) != id_to_book.end())
    {
        OrderBook *book = id_to_book.find(symbol_id)->second.get();
        book->deleteOrder(order_id);
    }
}

void OrderBookHandler::cancelOrder(uint32_t symbol_id, uint64_t order_id, uint64_t cancelled_quantity)
{
    if (id_to_book.find(symbol_id) != id_to_book.end())
    {
        OrderBook *book = id_to_book.find(symbol_id)->second.get();
        book->cancelOrder(order_id, cancelled_quantity);
    }
}

void OrderBookHandler::replaceOrder(uint32_t symbol_id, uint64_t order_id, uint64_t new_order_id, uint64_t new_price)
{
    if (id_to_book.find(symbol_id) != id_to_book.end())
    {
        OrderBook *book = id_to_book.find(symbol_id)->second.get();
        book->replaceOrder(order_id, new_order_id, new_price);
    }
}

void OrderBookHandler::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint64_t price)
{
    if (id_to_book.find(symbol_id) != id_to_book.end())
    {
        OrderBook *book = id_to_book.find(symbol_id)->second.get();
        book->executeOrder(order_id, quantity, price);
    }
}
void OrderBookHandler::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity)
{
    if (id_to_book.find(symbol_id) != id_to_book.end())
    {
        OrderBook *book = id_to_book.find(symbol_id)->second.get();
        book->executeOrder(order_id, quantity);
    }
}

std::string OrderBookHandler::toString()
{
    std::string book_handler_string;
    for (const auto &[symbol_id, book_ptr] : id_to_book)
    {
        if (book_ptr)
            book_handler_string += book_ptr->toString() + "\n";
    }
    return book_handler_string;
}

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

void Market::deleteSymbol(uint32_t symbol_id)
{
    if (id_to_symbol.find(symbol_id) != id_to_symbol.end())
    {
        id_to_symbol.erase(symbol_id);
        orderbook_handler->deleteOrderBook(symbol_id);
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