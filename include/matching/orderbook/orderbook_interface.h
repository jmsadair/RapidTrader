#ifndef FAST_EXCHANGE_ORDERBOOK_INTERFACE_H
#define FAST_EXCHANGE_ORDERBOOK_INTERFACE_H
#include <exception>
#include "types.h"
#include "command.h"

namespace OrderBook {
class OrderBook
{
public:
    /**
     * Places a new order.
     *
     * @param command an order.
     */
    virtual void placeOrder(Order &order) = 0;

    /**
     * Cancels an order.
     *
     * @param order_id the ID of the order to cancel.
     */
    virtual void cancelOrder(uint64_t order_id) = 0;

    /**
     * Reduces the order by the provided quantity.
     *
     * @param order_id the ID of order to reduce, require that there exist an order
     *                 in the book with order_id.
     * @param quantity_to_reduce_by the quantity to reduce the order by, require that
     *                              quantity_to_reduce by is less than the the current
     *                              quantity of the order.
     */
    virtual void reduceOrder(uint64_t order_id, uint64_t quantity_to_reduce_by) = 0;

    /**
     * Indicates whether an order is in the order book or not.
     *
     * @param order the order that may or may not be in the order book.
     * @return true if the book contains the order and false otherwise.
     */
    [[nodiscard]] virtual bool hasOrder(uint64_t order_id) const = 0;

    /**
     * Given an order ID, retrieves the corresponding order.
     *
     * @param order_id the ID associated with the order, require that
     *                 the order book contains an order with order_id.
     * @return the order with the provided ID.
     */
    [[nodiscard]] virtual const Order &getOrder(uint64_t order_id) const = 0;

    /**
     * @return the minimum asking price in the order book.
     */
    [[nodiscard]] virtual uint32_t minAskPrice() const = 0;

    /**
     * @return the maximum bidding price in the order book.
     */
    [[nodiscard]] virtual uint32_t maxBidPrice() const = 0;

    /**
     * A destructor for the OrderBook ADT.
     */
    virtual ~OrderBook() = default;

private:
    /**
     * Given a newly placed order command and an existing order fills each
     * as much as possible. Mutates the orders and potentially the matching.
     * Require that the orders are on opposite sides of the book.
     *
     * @param incoming the newly placed order that.
     * @param existing an order that already exists in the order book.
     */
    virtual void execute(Order &incoming, Order &existing) = 0;

    /**
     * Given a vector of prices that the provided order can match with, executes the orders
     * at those price levels until the provided order is fully filled. Mutates the order book
     * and the order.
     *
     * @param price_chain a vector of prices. Require that the vector is non-empty. If the order
     *                    is an ask order, require that the prices are strictly decreasing. If the
     *                    order is a bid order, require that the prices are strictly increasing.
     *                    Require that all prices correspond to a non-empty order list on the opposite
     *                    side of the book as the order (i.e. if the order is an ask order, the prices
     *                    should correspond to non-empty price levels on the bid side).
     * @param order an order to match with.
     */
    virtual void executePriceChain(const std::vector<uint32_t> &price_chain, Order &order) = 0;

    /**
     * Retrieves the prices that an order can match with and indicates whether
     * the order can be filled immediately or not.
     *
     * @param order an order.
     * @return a pair containing a boolean indicating whether the order can
     *         be executed in full immediately and a vector containing all of the
     *         prices that the order is able to match with. If the order is an ask
     *         order, then the prices in the vector are strictly increasing. Otherwise,
     *         if the order is a bid order, then the prices in the vector are strictly
     *         decreasing. Does not mutate the order book or the order.
     */
    virtual std::pair<std::vector<uint32_t>, bool> getPriceChain(const Order &order) = 0;

    /**
     * Fills an order as fully as possible.
     * Mutates the order.
     *
     * @param order an order to match.
     */
    virtual void match(Order &order) = 0;

    /**
     * Handles a GTC order.
     *
     * @param order a GTC order.
     */
    virtual void placeGtcOrder(Order &order) = 0;

    /**
     * Handles a FOK order.
     *
     * @param order a FOK order.
     */
    virtual void placeFokOrder(Order &order) = 0;

    /**
     * Handles an IOC order.
     *
     * @param order an IOC order.
     */
    virtual void placeIocOrder(Order &order) = 0;

    /**
     * Inserts an order into the order book.
     *
     * @param order the order to insert into the order book.
     */
    virtual void insert(const Order &order) = 0;

    /**
     * Removes an order from the order book if it exists.
     *
     * @param order the order to remove.
     */
    virtual void remove(const Order &order) = 0;
};
} // namespace OrderBook
#endif // FAST_EXCHANGE_ORDERBOOK_INTERFACE_H
