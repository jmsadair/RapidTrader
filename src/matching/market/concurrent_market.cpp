#include "market/concurrent_market.h"
#include "map_orderbook.h"

namespace RapidTrader::Matching {
ConcurrentMarket::ConcurrentMarket(std::vector<std::unique_ptr<EventHandler>> &event_handlers, uint8_t num_threads)
    : thread_pool(num_threads)
    , symbol_submission_index(0)
{
    orderbook_handlers.reserve(num_threads);
    for (uint32_t i = 0; i < num_threads; ++i)
        orderbook_handlers.push_back(std::make_unique<OrderBookHandler>(std::move(event_handlers[i])));
}

void ConcurrentMarket::addSymbol(uint32_t symbol_id, const std::string &symbol_name)
{
    if (id_to_symbol.find(symbol_id) == id_to_symbol.end())
    {
        id_to_symbol.insert({symbol_id, std::make_unique<Symbol>(symbol_id, symbol_name)});
        id_to_submission_index.insert({symbol_id, symbol_submission_index});
        OrderBookHandler *orderbook_handler = orderbook_handlers[symbol_submission_index].get();
        thread_pool.submitTask([=] { orderbook_handler->addOrderBook(symbol_id, symbol_name); }, symbol_submission_index);
        updateSymbolSubmissionIndex();
    }
}

void ConcurrentMarket::deleteSymbol(uint32_t symbol_id)
{
    if (id_to_symbol.find(symbol_id) != id_to_symbol.end())
    {
        uint32_t submission_index = getSubmissionIndex(symbol_id);
        id_to_symbol.erase(symbol_id);
        id_to_submission_index.erase(symbol_id);
        OrderBookHandler *orderbook_handler = orderbook_handlers[symbol_submission_index].get();
        thread_pool.submitTask([=] { orderbook_handler->deleteOrderBook(symbol_id); }, symbol_submission_index);
    }
}

void ConcurrentMarket::addOrder(const Order &order)
{
    uint32_t submission_index = getSubmissionIndex(order.getSymbolID());
    OrderBookHandler *orderbook_handler = orderbook_handlers[submission_index].get();
    thread_pool.submitTask([=] { orderbook_handler->addOrder(order); }, submission_index);
}

void ConcurrentMarket::deleteOrder(uint32_t symbol_id, uint64_t order_id)
{
    uint32_t submission_index = getSubmissionIndex(symbol_id);
    OrderBookHandler *orderbook_handler = orderbook_handlers[submission_index].get();
    thread_pool.submitTask([=] { orderbook_handler->deleteOrder(symbol_id, order_id); }, submission_index);
}

void ConcurrentMarket::cancelOrder(uint32_t symbol_id, uint64_t order_id, uint64_t cancelled_quantity)
{
    uint32_t submission_index = getSubmissionIndex(symbol_id);
    OrderBookHandler *orderbook_handler = orderbook_handlers[submission_index].get();
    thread_pool.submitTask([=] { orderbook_handler->cancelOrder(symbol_id, order_id, cancelled_quantity); }, submission_index);
}

void ConcurrentMarket::replaceOrder(uint32_t symbol_id, uint64_t order_id, uint64_t new_order_id, uint64_t new_price)
{
    uint32_t submission_index = getSubmissionIndex(symbol_id);
    OrderBookHandler *orderbook_handler = orderbook_handlers[submission_index].get();
    thread_pool.submitTask([=] { orderbook_handler->replaceOrder(symbol_id, order_id, new_order_id, new_price); }, submission_index);
}

void ConcurrentMarket::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint64_t price)
{
    uint32_t submission_index = getSubmissionIndex(symbol_id);
    OrderBookHandler *orderbook_handler = orderbook_handlers[submission_index].get();
    thread_pool.submitTask([=] { orderbook_handler->executeOrder(symbol_id, order_id, quantity, price); }, submission_index);
}

void ConcurrentMarket::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity)
{
    uint32_t submission_index = getSubmissionIndex(symbol_id);
    OrderBookHandler *orderbook_handler = orderbook_handlers[submission_index].get();
    thread_pool.submitTask([=] { orderbook_handler->executeOrder(symbol_id, order_id, quantity); }, submission_index);
}

uint32_t ConcurrentMarket::getSubmissionIndex(uint32_t symbol_id)
{
    auto it = id_to_submission_index.find(symbol_id);
    return it == id_to_submission_index.end() ? 0 : it->second;
}

void ConcurrentMarket::updateSymbolSubmissionIndex()
{
    symbol_submission_index = (symbol_submission_index + 1) % orderbook_handlers.size();
}
} // namespace RapidTrader::Matching