#include "concurrent_market.h"
#include "map_orderbook.h"

RapidTrader::Matching::ConcurrentMarket::ConcurrentMarket(Concurrent::Messaging::Sender outgoing_messages_, uint8_t num_threads)
    : outgoing_messages(outgoing_messages_)
    , thread_pool(num_threads)
    , new_symbol_queue_id(0)
{}
void RapidTrader::Matching::ConcurrentMarket::addSymbol(uint32_t symbol_id, const std::string &symbol_name)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
    {
        outgoing_messages.send(SymbolAdded{symbol_id, symbol_name});
        id_to_symbol.insert({symbol_id, std::make_unique<Symbol>(symbol_id, symbol_name)});
        id_to_book.insert({symbol_id, std::make_unique<MapOrderBook>(symbol_id, outgoing_messages)});
        id_to_queue_id.insert({symbol_id, new_symbol_queue_id});
        new_symbol_queue_id = (new_symbol_queue_id + 1) % thread_pool.numberOfThreads();
    }
}

void RapidTrader::Matching::ConcurrentMarket::addOrder(const Order &order)
{
    auto it = id_to_book.find(order.getSymbolID());
    OrderBook *book = it != id_to_book.end() ? it->second.get() : nullptr;
    uint8_t queue_id = it != id_to_book.end() ? id_to_queue_id[order.getSymbolID()] : 0;
    thread_pool.submitTask(
        [book, order] {
            if (book)
                book->addOrder(order);
        },
        queue_id);
}
void RapidTrader::Matching::ConcurrentMarket::deleteOrder(uint32_t symbol_id, uint64_t order_id)
{
    auto it = id_to_book.find(symbol_id);
    OrderBook *book = it != id_to_book.end() ? it->second.get() : nullptr;
    uint8_t queue_id = it != id_to_book.end() ? id_to_queue_id[symbol_id] : 0;
    thread_pool.submitTask(
        [book, order_id] {
            if (book)
                book->deleteOrder(order_id);
        },
        queue_id);
}
void RapidTrader::Matching::ConcurrentMarket::cancelOrder(uint32_t symbol_id, uint64_t order_id, uint64_t cancelled_quantity)
{
    auto it = id_to_book.find(symbol_id);
    OrderBook *book = it != id_to_book.end() ? it->second.get() : nullptr;
    uint8_t queue_id = it != id_to_book.end() ? id_to_queue_id[symbol_id] : 0;
    thread_pool.submitTask(
        [book, order_id, cancelled_quantity] {
            if (book)
                book->cancelOrder(order_id, cancelled_quantity);
        },
        queue_id);
}
void RapidTrader::Matching::ConcurrentMarket::replaceOrder(uint32_t symbol_id, uint64_t order_id, uint64_t new_order_id, uint64_t new_price)
{
    auto it = id_to_book.find(symbol_id);
    OrderBook *book = it != id_to_book.end() ? it->second.get() : nullptr;
    uint8_t queue_id = it != id_to_book.end() ? id_to_queue_id[symbol_id] : 0;
    thread_pool.submitTask(
        [book, order_id, new_order_id, new_price] {
            if (book)
                book->replaceOrder(order_id, new_order_id, new_price);
        },
        queue_id);
}
void RapidTrader::Matching::ConcurrentMarket::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint64_t price)
{
    auto it = id_to_book.find(symbol_id);
    OrderBook *book = it != id_to_book.end() ? it->second.get() : nullptr;
    uint8_t queue_id = it != id_to_book.end() ? id_to_queue_id[symbol_id] : 0;
    thread_pool.submitTask(
        [book, order_id, quantity, price] {
            if (book)
                book->executeOrder(order_id, quantity, price);
        },
        queue_id);
}
void RapidTrader::Matching::ConcurrentMarket::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity)
{
    auto it = id_to_book.find(symbol_id);
    OrderBook *book = it != id_to_book.end() ? it->second.get() : nullptr;
    uint8_t queue_id = it != id_to_book.end() ? id_to_queue_id[symbol_id] : 0;
    thread_pool.submitTask(
        [book, order_id, quantity] {
            if (book)
                book->executeOrder(order_id, quantity);
        },
        queue_id);
}
