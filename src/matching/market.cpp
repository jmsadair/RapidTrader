#include "market.h"
#include "map_orderbook.h"
#include "event_handler/event.h"

namespace RapidTrader::Matching {

#ifndef CONCURRENT
Market::Market(Concurrent::Messaging::Sender outgoing_messages_)
    : outgoing_messages(outgoing_messages_)
{}
#endif

#ifdef CONCURRENT
Market::Market(Concurrent::Messaging::Sender outgoing_messages_, uint8_t num_threads_)
    : outgoing_messages(outgoing_messages_)
    , thread_pool(num_threads_)
    , new_symbol_queue_id(0)
{}
#endif

ErrorStatus Market::addSymbol(uint32_t symbol_id, std::string symbol_name)
{
    if (id_to_symbol.find(symbol_id) != id_to_symbol.end())
        return ErrorStatus::DuplicateSymbol;
    outgoing_messages.send(SymbolAdded{symbol_id, symbol_name});
    id_to_symbol.insert({symbol_id, std::move(symbol_name)});
    return ErrorStatus::Ok;
}

ErrorStatus Market::deleteSymbol(uint32_t symbol_id)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
        return ErrorStatus::SymbolDoesNotExist;
    outgoing_messages.send(SymbolDeleted{symbol_id, id_to_symbol[symbol_id]});
    // Delete the orderbook associated with the symbol if necessary.
    if (symbol_id < symbol_to_book.size() && symbol_to_book[symbol_id])
        symbol_to_book[symbol_id] = nullptr;
    // Delete the symbol.
    id_to_symbol.erase(symbol_id);
#ifdef CONCURRENT
    id_to_queue_id.erase(symbol_id);
#endif
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
    outgoing_messages.send(OrderBookAdded{symbol_id});
    symbol_to_book[symbol_id] = std::make_unique<MapOrderBook>(symbol_id, outgoing_messages);
#ifdef CONCURRENT
    id_to_queue_id.insert({symbol_id, new_symbol_queue_id});
    new_symbol_queue_id = (new_symbol_queue_id + 1) % thread_pool.numberOfThreads();
#endif
    return ErrorStatus::Ok;
}

ErrorStatus Market::deleteOrderbook(uint32_t symbol_id)
{
    if (symbol_id >= symbol_to_book.size() || !symbol_to_book[symbol_id])
        return ErrorStatus::OrderBookDoesNotExist;
    symbol_to_book[symbol_id] = nullptr;
    outgoing_messages.send(OrderBookDeleted{symbol_id});
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
#ifndef CONCURRENT
    book->addOrder(order);
#else
    thread_pool.submitTask(id_to_queue_id[order.getSymbolID()], [book, order] { book->addOrder(order); });
#endif
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
#ifndef CONCURRENT
    book->deleteOrder(order_id);
#else
    thread_pool.submitTask(id_to_queue_id[symbol_id], [book, order_id] { book->deleteOrder(order_id); });
#endif
    return ErrorStatus::Ok;
}

ErrorStatus Market::cancelOrder(uint32_t symbol_id, uint64_t order_id, uint64_t cancelled_quantity)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
        return ErrorStatus::SymbolDoesNotExist;
    if (symbol_id >= symbol_to_book.size() || !symbol_to_book[symbol_id])
        return ErrorStatus::OrderBookDoesNotExist;
    if (cancelled_quantity == 0)
        return ErrorStatus::InvalidQuantity;
    OrderBook *book = symbol_to_book[symbol_id].get();
    // Check that the orderbook has the order.
    if (!book->hasOrder(order_id))
        return ErrorStatus::OrderDoesNotExist;
#ifndef CONCURRENT
    book->cancelOrder(order_id, cancelled_quantity);
#else
    thread_pool.submitTask(id_to_queue_id[symbol_id], [book, order_id, cancelled_quantity] { book->cancelOrder(order_id, cancelled_quantity); });
#endif
    return ErrorStatus::Ok;
}
ErrorStatus Market::replaceOrder(uint32_t symbol_id, uint64_t order_id, uint64_t new_order_id, uint64_t new_price)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
        return ErrorStatus::SymbolDoesNotExist;
    if (symbol_id >= symbol_to_book.size() || !symbol_to_book[symbol_id])
        return ErrorStatus::OrderBookDoesNotExist;
    if (new_price <= 0)
        return ErrorStatus::InvalidPrice;
    if (new_order_id == 0)
        return ErrorStatus::InvalidOrderID;
    OrderBook *book = symbol_to_book[symbol_id].get();
    // Check that the orderbook has the order.
    if (!book->hasOrder(order_id))
        return ErrorStatus::OrderDoesNotExist;
#ifndef CONCURRENT
    book->replaceOrder(order_id, new_order_id, new_price);
#else
    thread_pool.submitTask(id_to_queue_id[symbol_id], [book, order_id, new_order_id, new_price] { book->replaceOrder(order_id, new_order_id, new_price); });
#endif
    return ErrorStatus::Ok;
}

ErrorStatus Market::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint64_t price)
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
#ifndef CONCURRENT
    book->executeOrder(order_id, quantity, price);
#else
    thread_pool.submitTask(id_to_queue_id[symbol_id], [book, order_id, quantity, price] { book->executeOrder(order_id, quantity, price);});
#endif
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
#ifndef CONCURRENT
    book->executeOrder(order_id, quantity);
#else
    thread_pool.submitTask(id_to_queue_id[symbol_id], [book, order_id, quantity] { book->executeOrder(order_id, quantity); });
#endif
    return ErrorStatus::Ok;
}
} // namespace RapidTrader::Matching