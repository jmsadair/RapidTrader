#ifndef RAPID_TRADER_MARKET_H
#define RAPID_TRADER_MARKET_H
#include <iostream>
#include <robin_hood.h>
#include <memory>
#include "order.h"
#include "sender.h"
#include "orderbook.h"

enum class ErrorStatus {
    Ok,
    DuplicateSymbol,
    DuplicateOrder,
    DuplicateOrderBook,
    SymbolDoesNotExist,
    OrderBookDoesNotExist,
    OrderDoesNotExist,
    InvalidQuantity,
    InvalidPrice
};

namespace RapidTrader::Matching {
class Market
{
public:
    Market(Market &&other) = delete;
    Market &operator=(Market &&other) = delete;

    /**
     * A constructor for the Market.
     *
     * @param outgoing_messenger_ a message queue for outgoing messages.
     */
    explicit Market(Messaging::Sender outgoing_messenger_);

    /**
     * Adds a new symbol to market.
     *
     * @param symbol_id the ID that the symbol is identified by, require that
     *                  the symbol associated with symbol ID does not already exist.
     * @param symbol_name the name of the symbol.
     * @return ErrorStatus indicating if the symbol was added successfully.
     */
    ErrorStatus addSymbol(uint32_t symbol_id, std::string symbol_name);

    /**
     * Deletes an existing symbol from the market.
     *
     * @param symbol_id the ID associated with the symbol to delete, require that
     *                  the symbol exists in the market.
     * @return
     */
    ErrorStatus deleteSymbol(uint32_t symbol_id);

    /**
     * @param symbol_id the ID of the symbol to check for the presence of.
     * @return true if the market has the symbol and false otherwise.
     */
    bool hasSymbol(uint32_t symbol_id) const;

    /**
     * Adds a new orderbook for the provided symbol to the market.
     *
     * @param symbol_id an ID associated with a symbol, require that the market has a
     *                  symbol associated with the ID.
     * @return ErrorStatus indicating whether the orderbook was successfully added to the market.
     */
    ErrorStatus addOrderbook(uint32_t symbol_id);

    /**
     * Deletes an orderbook from the market.
     *
     * @param symbol_id a symbol ID associated with an orderbook, require that there exists
     *                  an orderbook associated with the ID.
     * @return ErrorStatus indicating whether the orderbook was successfully deleted.
     */
    ErrorStatus deleteOrderbook(uint32_t symbol_id);

    /**
     * Retrieves an orderbook from the market.
     *
     * @param symbol_id a symbol ID that that the orderbook is associated with, require that
     *                  there exists an orderbook associated with the ID.
     * @return the orderbook.
     */
    const OrderBook& getOrderbook(uint32_t symbol_id) const;

    /**
     * Indicates whether an orderbook is present in the market.
     *
     * @param symbol_id the symbol ID associated with the orderbook (that may not exist).
     * @return true if the orderbook exists and false otherwise.
     */
    bool hasOrderbook(uint32_t symbol_id) const;

    /**
     * Submits a new order to the market.
     *
     * @param order the order to submit.
     * @return ErrorStatus indicating whether the order was successfully added.
     */
    ErrorStatus addOrder(const Order &order);

    /**
     * Deletes an existing order from the market, require that the order exists.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @return ErrorStatus indicating whether the order was successfully deleted.
     */
    ErrorStatus deleteOrder(uint32_t symbol_id, uint64_t order_id);

    /**
     * Cancels the provided quantity of an existing order in the market, require that the order exists.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param cancelled_quantity the quantity of the order to cancel, require that cancelled_quantity is positive.
     * @return ErrorStatus indicating whether the order was successfully cancelled.
     */
    ErrorStatus cancelOrder(uint32_t symbol_id, uint64_t order_id, uint64_t cancelled_quantity);

    /**
     * Replaces an existing order in the market, require that the order exists.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param new_order_id the new ID to assign to the order.
     * @param new_price the new price to assign to the order, require that price is positive.
     * @return ErrorStatus indicating whether the order was successfully replaced.
     */
    ErrorStatus replaceOrder(uint32_t symbol_id, uint64_t order_id, uint64_t new_order_id, uint32_t new_price);

    /**
     * Executes an existing order in the market.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param quantity the quantity of the order to execute, require that quantity is positive.
     * @param price the price at which the order is executed, require that price is positive.
     * @return ErrorStatus indicating whether the order order was successfully executed.
     */
    ErrorStatus executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint32_t price);

    /**
     * Executes an existing order in the market.
     *
     * @param symbol_id the symbol ID associated with the order.
     * @param order_id the ID associated with the order.
     * @param quantity the quantity of the order to execute, require that quantity is positive.
     * @return ErrorStatus indicating whether the order was successfully executed.
     */
    ErrorStatus executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity);
private:
    // Maps symbol to orderbook.
    std::vector<std::unique_ptr<OrderBook>> symbol_to_book;
    // Symbol IDs to symbol names.
    std::unordered_map<uint32_t, std::string> id_to_symbol;
    // Sends outgoing messages regarding the statuses of orders.
    Messaging::Sender outgoing_messenger;
};
} // namespace RapidTrader::Matching
#endif // RAPID_TRADER_MARKET_H
