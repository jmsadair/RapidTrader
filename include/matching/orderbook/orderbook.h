#ifndef RAPID_TRADER_ORDERBOOK_H
#define RAPID_TRADER_ORDERBOOK_H
#include "order.h"

class OrderBook
{
public:
    /**
     * Submits an order to order book.
     *
     * @param order the order to submit.
     */
    virtual void addOrder(Order order) = 0;

    /**
     * Executes an order in the book.
     *
     * @param order_id the ID of the order to execute, require that an order with the provided
     *                 ID exists in the book.
     * @param quantity the quantity of the order to execute, require that quantity is positive.
     * @param price the price to execute the order at, require that price is positive.
     */
    virtual void executeOrder(uint64_t order_id, uint64_t quantity, uint64_t price) = 0;

    /**
     * Executes an order in the book.
     *
     * @param order_id the ID of the order to execute, require that an order with the provided
     *                 ID exists in the book.
     * @param quantity the quantity of the order to execute, require that quantity is positive.
     */
    virtual void executeOrder(uint64_t order_id, uint64_t quantity) = 0;

    /**
     * Deletes an existing order from the book.
     *
     * @param order_id the ID of the order to delete, require that an order with the provided
     *                 ID exists in the book.
     */
    virtual void deleteOrder(uint64_t order_id) = 0;

    /**
     * Cancels the provided quantity of an order in the book. Removes order from
     * the book if provided quantity exceeds the open quantity of the order.
     *
     * @param order_id the ID of the order to cancel, require that an order with the provided
     *                 ID exists in the book.
     * @param quantity the quantity of the order to cancel, require that quantity is positive.
     */
    virtual void cancelOrder(uint64_t order_id, uint64_t quantity) = 0;

    /**
     * Replaces an existing order in the book.
     *
     * @param order_id the ID of the order to replace, require that an order with the provided
     *                 ID exists in the book.
     * @param new_order_id the ID that the new order will have.
     * @param new_price the new price of the order, require that new_price is positive.
     */
    virtual void replaceOrder(uint64_t order_id, uint64_t new_order_id, uint64_t new_price) = 0;

    /**
     * @param order_id the ID of the order to check the book for, require that quantity is positive.
     * @return true if the order is in the book and false otherwise.
     */
    [[nodiscard]] virtual bool hasOrder(uint64_t order_id) const = 0;

    /**
     * Retrieves an order from the book.
     *
     * @param order_id the ID of the order being requested, require that an order with
     *                 the provided ID exists in the book.
     * @return the order with provided ID.
     */
    [[nodiscard]] virtual const Order &getOrder(uint64_t order_id) const = 0;

    /**
     * @return true if there are no orders in the book and false otherwise.
     */
    [[nodiscard]] virtual bool empty() const = 0;

    /**
     * @return a string representation of the book.
     */
    [[nodiscard]] virtual std::string toString() const = 0;

    virtual ~OrderBook() = default;
};
#endif // RAPID_TRADER_ORDERBOOK_H
