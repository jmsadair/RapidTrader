#ifndef FAST_EXCHANGE_ORDERBOOK_INTERFACE_H
#define FAST_EXCHANGE_ORDERBOOK_INTERFACE_H
#include <string>
#include <exception>
#include "types.h"
#include "price_level.h"
#include "commands.h"
#include "results.h"
#include "receiver.h"

namespace OrderBook {
    class OrderBook {
    public:
        /**
         * Places a new order. Mutates the command.
         *
         * @param command a command to place an order.
         */
        virtual void placeOrder(PlaceOrderCommand &command) = 0;

        /**
         * Cancels an order.
         *
         * @param command a command to cancel an order.
         */
        virtual void cancelOrder(CancelOrderCommand &command) = 0;

        /**
         * Indicates whether an order is in the order book or not.
         *
         * @param order the order that may or may not be in the order book.
         * @return true if the book contains the order and false otherwise.
         */
        [[nodiscard]] virtual bool hasOrder(OrderID order_id) const = 0;

        /**
         * Given an order ID, retrieves the corresponding order.
         *
         * @param order_id the ID associated with the order, require that
         *                 the order book contains an order with order_id.
         * @return the order with the provided ID.
         */
        [[nodiscard]] virtual const Order& getOrder(OrderID order_id) const = 0;

        /**
         * Converts the order book to its string representation.
         *
         * @return the string representation of the order book.
         */
        [[nodiscard]] virtual std::string toString() const = 0;

        /**
         * A destructor for the OrderBook ADT.
         */
        virtual ~OrderBook() = default;

    private:
        /**
         * Executes an order command as fully as possible.
         * Mutates the command.
         *
         * @param command a command to place an order.
         * @return the quantity of the order that has not been filled.
         */
        virtual Quantity execute(PlaceOrderCommand &command) = 0;

        /**
         * Handles a GTC order.
         *
         * @param command a command to place a GTC order.
         */
        virtual void handleGtcOrder(PlaceOrderCommand &command) = 0;

        /**
         * Creates a new order and inserts it into the order book.
         *
         * @param order the order to insert into the order book.
         * @return a reference to the newly inserted order.
         */
        virtual const Order& insert(Order order) = 0;

        /**
         * Removes an order from the order book if it exists.
         *
         * @param order the order to remove.
         */
        virtual void remove(Order &order) = 0;
    };
}
#endif //FAST_EXCHANGE_ORDERBOOK_INTERFACE_H
