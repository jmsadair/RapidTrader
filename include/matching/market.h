#ifndef RAPID_TRADER_MARKET_H
#define RAPID_TRADER_MARKET_H
#include <iostream>
#include <fstream>
#include <robin_hood.h>
#include <memory>
#include "order.h"
#include "orderbook.h"
#include "event_handler/event_handler.h"
#include "concurrent/thread_pool/thread_pool.h"
#include "symbol.h"

namespace RapidTrader::Matching {
class Market
{
public:
    Market(Market &&other) = delete;
    Market &operator=(Market &&other) = delete;

    /**
     * A constructor for the Market.
     *
     * @param outgoing_messages_ sends updates from the market.
     */
    explicit Market(Concurrent::Messaging::Sender outgoing_messages_);

    /**
     * Adds a new symbol to market.
     *
     * @param symbol_id the ID that the symbol is identified by, require that
     *                  the symbol associated with symbol ID does not already exist.
     * @param symbol_name the name of the symbol.
     */
    void addSymbol(uint32_t symbol_id, const std::string &symbol_name);

    /**
     * @param symbol_id the ID of the symbol to check for the presence of.
     * @return true if the market has the symbol and false otherwise.
     */
    [[nodiscard]] bool hasSymbol(uint32_t symbol_id) const;

    /**
     * Retrieves an orderbook from the market.
     *
     * @param symbol_id a symbol ID that that the orderbook is associated with, require that
     *                  there exists an orderbook associated with the ID.
     * @return the orderbook.
     */
    [[nodiscard]] const OrderBook &getOrderbook(uint32_t symbol_id) const;

    /**
     * Indicates whether an orderbook is present in the market.
     *
     * @param symbol_id the symbol ID associated with the orderbook (that may not exist).
     * @return true if the orderbook exists and false otherwise.
     */
    [[nodiscard]] bool hasOrderbook(uint32_t symbol_id) const;

    /**
     * Submits a new order to the market.
     *
     * @param order the order to submit.
     */
    void addOrder(const Order &order);

    /**
     * Deletes an existing order from the market, require that the order exists.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     */
    void deleteOrder(uint32_t symbol_id, uint64_t order_id);

    /**
     * Cancels the provided quantity of an existing order in the market, require that the order exists.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param cancelled_quantity the quantity of the order to cancel, require that cancelled_quantity is positive.
     */
    void cancelOrder(uint32_t symbol_id, uint64_t order_id, uint64_t cancelled_quantity);

    /**
     * Replaces an existing order in the market, require that the order exists.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param new_order_id the new ID to assign to the order.
     * @param new_price the new price to assign to the order, require that price is positive.
     */
    void replaceOrder(uint32_t symbol_id, uint64_t order_id, uint64_t new_order_id, uint64_t new_price);

    /**
     * Executes an existing order in the market.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param quantity the quantity of the order to execute, require that quantity is positive.
     * @param price the price at which the order is executed, require that price is positive.
     */
    void executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint64_t price);

    /**
     * Executes an existing order in the market.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param quantity the quantity of the order to execute, require that quantity is positive.
     */
    void executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity);

    /**
     * @return the string representation of the market.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * Writes the string representation of the the market to
     * a file at the provided path. Creates a new file.
     *
     * @param path the path to the file that will be written to.
     */
    void dumpMarket(const std::string &path) const;

    friend std::ostream &operator<<(std::ostream &os, const Market &book);

private:
    // Maps symbol to orderbook.
    robin_hood::unordered_map<uint32_t, std::unique_ptr<OrderBook>> id_to_book;
    // Symbol IDs to symbol names.
    robin_hood::unordered_map<uint32_t, std::unique_ptr<Symbol>> id_to_symbol;
    // Sends updates from market to event handler.
    Concurrent::Messaging::Sender outgoing_messages;
};
} // namespace RapidTrader::Matching
#endif // RAPID_TRADER_MARKET_H
