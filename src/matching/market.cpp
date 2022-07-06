#include "market.h"
#include "map_orderbook.h"
#include "notification.h"
#include "log.h"

namespace RapidTrader::Matching {
Market::Market(Messaging::Sender outgoing_messenger_)
    : outgoing_messenger(outgoing_messenger_)
{}

ErrorStatus Market::addSymbol(uint32_t symbol_id, std::string symbol_name)
{
    if (id_to_symbol.find(symbol_id) != id_to_symbol.end())
        return ErrorStatus::DuplicateSymbol;
    outgoing_messenger.send(AddedSymbol{symbol_id, symbol_name});
    id_to_symbol.insert({symbol_id, std::move(symbol_name)});
    return ErrorStatus::Ok;
}

ErrorStatus Market::deleteSymbol(uint32_t symbol_id)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
        return ErrorStatus::SymbolDoesNotExist;
    outgoing_messenger.send(DeletedSymbol{symbol_id, id_to_symbol[symbol_id]});
    symbol_to_book[symbol_id] = nullptr;
    id_to_symbol.erase(symbol_id);
    return ErrorStatus::Ok;
}

bool Market::hasSymbol(uint32_t symbol_id) const
{
    return id_to_symbol.find(symbol_id) != id_to_symbol.end();
}

ErrorStatus Market::addOrderbook(uint32_t symbol_id)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
        return ErrorStatus::SymbolDoesNotExist;
    if (symbol_id < symbol_to_book.size() && symbol_to_book[symbol_id])
        return ErrorStatus::DuplicateOrderBook;
    if (symbol_id >= symbol_to_book.size())
        symbol_to_book.resize(symbol_id + 1);
    symbol_to_book[symbol_id] = std::make_unique<MapOrderBook>(symbol_id, outgoing_messenger);
    outgoing_messenger.send(AddedOrderBook{symbol_id});
    return ErrorStatus::Ok;
}

ErrorStatus Market::deleteOrderbook(uint32_t symbol_id)
{
    if (symbol_id >= symbol_to_book.size() || !symbol_to_book[symbol_id])
        return ErrorStatus::OrderBookDoesNotExist;
    symbol_to_book[symbol_id] = nullptr;
    outgoing_messenger.send(DeletedOrderBook{symbol_id});
    return ErrorStatus::Ok;
}

bool Market::hasOrderbook(uint32_t symbol_id) const
{
    return symbol_id < symbol_to_book.size() && symbol_to_book[symbol_id];
}

const OrderBook &Market::getOrderbook(uint32_t symbol_id) const
{
    return *symbol_to_book[symbol_id].get();
}

ErrorStatus Market::addOrder(const Order &order)
{
    if (id_to_symbol.find(order.getSymbolID()) == id_to_symbol.end())
        return ErrorStatus::SymbolDoesNotExist;
    if (order.getSymbolID() >= symbol_to_book.size() || !symbol_to_book[order.getSymbolID()])
        return ErrorStatus::OrderBookDoesNotExist;
    // Get orderbook that corresponds to order symbol ID.
    OrderBook *book = symbol_to_book[order.getSymbolID()].get();
    // Make sure order does not already exist.
    if (book->hasOrder(order.getOrderID()))
        return ErrorStatus::DuplicateOrder;
    // Submit order for matching.
    switch(order.getAction())
    {
        case OrderAction::Limit:
            book->addLimitOrder(order);
            break;
        case OrderAction::Market:
            book->addMarketOrder(order);
            break;
    }
    return ErrorStatus::Ok;
}

ErrorStatus Market::deleteOrder(uint32_t symbol_id, uint64_t order_id)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
        return ErrorStatus::SymbolDoesNotExist;
    if (symbol_id >= symbol_to_book.size() || !symbol_to_book[symbol_id])
        return ErrorStatus::OrderBookDoesNotExist;
    OrderBook *book = symbol_to_book[symbol_id].get();
    // Check that the orderbook has the order.
    if (!book->hasOrder(order_id))
        return ErrorStatus::OrderDoesNotExist;
    book->deleteOrder(order_id);
    return ErrorStatus::Ok;
}

ErrorStatus Market::cancelOrder(uint32_t symbol_id, uint64_t order_id, uint64_t cancelled_quantity)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
        return ErrorStatus::SymbolDoesNotExist;
    if (symbol_id >= symbol_to_book.size() || !symbol_to_book[symbol_id])
        return ErrorStatus::OrderBookDoesNotExist;
    OrderBook *book = symbol_to_book[symbol_id].get();
    // Check that the orderbook has the order.
    if (!book->hasOrder(order_id))
        return ErrorStatus::OrderDoesNotExist;
    book->cancelOrder(order_id, cancelled_quantity);
    return ErrorStatus::Ok;
}
ErrorStatus Market::replaceOrder(uint32_t symbol_id, uint64_t order_id, uint64_t new_order_id, uint32_t new_price)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
        return ErrorStatus::SymbolDoesNotExist;
    if (symbol_id >= symbol_to_book.size() || !symbol_to_book[symbol_id])
        return ErrorStatus::OrderBookDoesNotExist;
    if (new_price <= 0)
        return ErrorStatus::InvalidPrice;
    OrderBook *book = symbol_to_book[symbol_id].get();
    // Check that the orderbook has the order.
    if (!book->hasOrder(order_id))
        return ErrorStatus::OrderDoesNotExist;
    // Find the order to replace.
    auto order_to_replace = book->getOrder(order_id);
    // Make a copy of the order with the modified ID and price, and reinsert into the book.
    Order new_order = order_to_replace;
    new_order.setOrderID(new_order_id);
    new_order.setPrice(new_price);
    book->deleteOrder(order_id);
    book->addLimitOrder(new_order);
    return ErrorStatus::Ok;
}

ErrorStatus Market::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint32_t price)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
        return ErrorStatus::SymbolDoesNotExist;
    if (symbol_id >= symbol_to_book.size() || !symbol_to_book[symbol_id])
        return ErrorStatus::OrderBookDoesNotExist;
    if (price <= 0)
        return ErrorStatus::InvalidPrice;
    if (quantity <= 0)
        return ErrorStatus::InvalidQuantity;
    OrderBook *book = symbol_to_book[symbol_id].get();
    // Check that the orderbook has the order.
    if (!book->hasOrder(order_id))
        return ErrorStatus::OrderDoesNotExist;
    book->executeOrder(order_id, quantity, price);
    return ErrorStatus::Ok;

}
ErrorStatus Market::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
        return ErrorStatus::SymbolDoesNotExist;
    if (symbol_id >= symbol_to_book.size() || !symbol_to_book[symbol_id])
        return ErrorStatus::OrderBookDoesNotExist;
    if (quantity <= 0)
        return ErrorStatus::InvalidQuantity;
    OrderBook *book = symbol_to_book[symbol_id].get();
    // Check that the orderbook has the order.
    if (!book->hasOrder(order_id))
        return ErrorStatus::OrderDoesNotExist;
    book->executeOrder(order_id, quantity);
    return ErrorStatus::Ok;
}
} // namespace RapidTrader::Matching