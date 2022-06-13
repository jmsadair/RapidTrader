#ifndef FAST_EXCHANGE_ORDERBOOK_INTERFACE_H
#define FAST_EXCHANGE_ORDERBOOK_INTERFACE_H
#include <exception>
#include "types.h"
#include "command.h"

namespace OrderBook {
    class OrderBook {
    public:
        /**
         * Places a new order.
         *
         * @param command an order.
         */
        virtual void placeOrder(Order order) = 0;

        /**
         * Cancels an order.
         *
         * @param order_id the ID of the order to cancel.
         */
        virtual void cancelOrder(uint64_t order_id) = 0;

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
        [[nodiscard]] virtual const Order& getOrder(uint64_t order_id) const = 0;

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
         * as much as possible. Mutates the orders and potentially the matching_engine.
         * Require that the orders are on opposite sides of the book.
         *
         * @param incoming the newly placed order that.
         * @param existing an order that already exists in the order book.
         */
        virtual void execute(Order &incoming, Order &existing) = 0;

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
        virtual void placeGtcOrder(Order order) = 0;

        /**
         * Handles a FOK order.
         *
         * @param order a FOK order.
         */
        virtual void placeFokOrder(Order order) = 0;

        /**
         * Inserts an order into the order book.
         *
         * @param order the order to insert into the order book.
         */
        virtual void insert(Order order) = 0;

        /**
         * Removes an order from the order book if it exists.
         *
         * @param order the order to remove.
         */
        virtual void remove(Order &order) = 0;
    };
}
#endif //FAST_EXCHANGE_ORDERBOOK_INTERFACE_H
