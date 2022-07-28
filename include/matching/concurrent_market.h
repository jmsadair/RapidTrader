#ifndef RAPID_TRADER_CONCURRENT_MARKET_H
#define RAPID_TRADER_CONCURRENT_MARKET_H
#include <iostream>
#include <fstream>
#include <robin_hood.h>
#include <memory>
#include <future>
#include "order.h"
#include "orderbook.h"
#include "event_handler/event_handler.h"
#include "concurrent/thread_pool/thread_pool.h"
#include "symbol.h"

namespace RapidTrader::Matching {
class ConcurrentMarket
{
public:
    ConcurrentMarket(ConcurrentMarket &&other) = delete;
    ConcurrentMarket &operator=(ConcurrentMarket &&other) = delete;

    /**
     * A constructor for the Market.
     *
     * @param outgoing_messages_ sends updates from the market.
     * @param num_threads the number of worker threads that will be used, require that
     *                    num_threads is positive.
     */
    explicit ConcurrentMarket(Concurrent::Messaging::Sender outgoing_messages_, uint8_t num_threads = std::thread::hardware_concurrency());


    /**
     * Adds a new symbol to market.
     *
     * @param symbol_id the ID that the symbol is identified by, require that
     *                  the symbol associated with symbol ID does not already exist.
     * @param symbol_name the name of the symbol.
     * @return ErrorStatus indicating if the symbol was added successfully.
     */
    void addSymbol(uint32_t symbol_id, const std::string& symbol_name);

    /**
     * Submits a new order to the market.
     *
     * @param order the order to submit.
     * @return ErrorStatus indicating whether the order was successfully added.
     */
    void addOrder(const Order &order);

    /**
     * Deletes an existing order from the market, require that the order exists.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @return ErrorStatus indicating whether the order was successfully deleted.
     */
    void deleteOrder(uint32_t symbol_id, uint64_t order_id);

    /**
     * Cancels the provided quantity of an existing order in the market, require that the order exists.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param cancelled_quantity the quantity of the order to cancel, require that cancelled_quantity is positive.
     * @return ErrorStatus indicating whether the order was successfully cancelled.
     */
    void cancelOrder(uint32_t symbol_id, uint64_t order_id, uint64_t cancelled_quantity);

    /**
     * Replaces an existing order in the market, require that the order exists.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param new_order_id the new ID to assign to the order.
     * @param new_price the new price to assign to the order, require that price is positive.
     * @return ErrorStatus indicating whether the order was successfully replaced.
     */
    void replaceOrder(uint32_t symbol_id, uint64_t order_id, uint64_t new_order_id, uint64_t new_price);

    /**
     * Executes an existing order in the market.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param quantity the quantity of the order to execute, require that quantity is positive.
     * @param price the price at which the order is executed, require that price is positive.
     * @return ErrorStatus indicating whether the order order was successfully executed.
     */
    void executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint64_t price);

    /**
     * Executes an existing order in the market.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param quantity the quantity of the order to execute, require that quantity is positive.
     * @return ErrorStatus indicating whether the order was successfully executed.
     */
    void executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity);

private:
    // Maps symbol IDs to order books.
    robin_hood::unordered_map<uint32_t, std::unique_ptr<OrderBook>> id_to_book;
    // Maps symbol IDs to symbols.
    robin_hood::unordered_map<uint32_t, std::unique_ptr<Symbol>> id_to_symbol;
    // Map symbol IDs to thread pool queue IDs.
    robin_hood::unordered_map<uint32_t, uint8_t> id_to_queue_id;
    // Sends updates from market to event handler.
    Concurrent::Messaging::Sender outgoing_messages;
    // Thread pool for task submission.
    Concurrent::ThreadPool thread_pool;
    // The queue ID that a newly added symbol will be associated with.
    uint8_t new_symbol_queue_id;
};
} // namespace RapidTrader::Matching
#endif // RAPID_TRADER_CONCURRENT_MARKET_H
