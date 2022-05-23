#ifndef FAST_EXCHANGE_ORDER_BOOK_H
#define FAST_EXCHANGE_ORDER_BOOK_H
#include <map>
#include <string>
#include <decimal/decimal>
#include "order_list.h"

namespace OrderBook {
    using OrderID = uint64_t;
    class OrderBook {
    public:
        /**
         * A constructor for the OrderBook ADT.
         *
         * @param order_book_symbol the symbol associated with the order book, require that
         *                          order_book_symbol is an arrangement of characters that represents
         *                          a publicly traded security.
         */
        explicit OrderBook(std::string order_book_symbol) : symbol(std::move(order_book_symbol)) {}

        /**
         * Places a new order.
         *
         * @param order the order to place.
         */
        void placeOrder(Order& order) {}

        /**
         * Cancels the order if it exists in the order book.
         *
         * @param order the order to cancel.
         */
        void cancelOrder(Order& order) {}

        /**
         * Indicates whether the provided order is present in the
         * order book.
         *
         * @param order an order.
         * @return true if the order is in the order book and false
         *         otherwise.
         */
        bool hasOrder(Order& order) {}

    private:
        /**
         * Attempts to execute an order as fully as possible.
         *
         * @param order the order to execute.
         */
        void tryMatch(Order& order) {}

        /**
        * Places a GTC (Good 'Till Cancel) order.
        *
        * @param order a GTC order.
        */
        void handleGtcOrder(Order& order) {}

        /**
         * Places a FOK (Fill or Kill Order). Note that this order will
         * not be added to the order book - if possible, the order will
         * be executed immediately in its entirety; otherwise, it will
         * be cancelled.
         *
         * @param order a FOK order.
         */
        void handleFokOrder(Order& order) {}

        /**
         * Places an IOC (Immediate or Cancel Order). Note that order will
         * not be added to the order book - the order will be executed immediately
         * as fully as possible and non-executed parts of the order are deleted
         * without entry into the order book.
         *
         * @param order a IOC order.
         */
        void handleIocOrder(Order& order) {}

        /**
         * Removes the order from the order book, if its exists.
         *
         * @param order the order to remove.
         */
        inline void removeOrder(Order& order) {
            auto& order_side_map = order.side == OrderSide::Ask ? ask_map : bid_map;
            order_side_map.at(order.price).removeOrder(order);
            order_map.erase(order.id);
        }

        /**
         * Inserts an order into the order book.
         *
         * @param order the order to insert.
         */
        inline void insertOrder(Order& order) {
            auto& order_side_map = order.side == OrderSide::Ask ? ask_map : bid_map;
            auto pair = order_side_map.emplace(order.price, order);
            if (!pair.second) {pair.first->second.addOrder(order); }
            order_map.insert(std::make_pair(order.id, order));
        }

        std::map<Price, OrderList> bid_map;
        std::map<Price, OrderList> ask_map;
        std::unordered_map<OrderID, Order> order_map;
        std::string symbol;
    };
}
#endif //FAST_EXCHANGE_ORDER_BOOK_H
