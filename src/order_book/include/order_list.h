#ifndef FAST_EXCHANGE_ORDER_LIST_H
#define FAST_EXCHANGE_ORDER_LIST_H
#include <list>
#include <unordered_map>
#include <algorithm>
#include "order.h"

using namespace boost::intrusive;

/**
 * A doubly linked list of orders that are associated with a single price.
 */
namespace OrderBook {
    class OrderList {
    public:
        /**
         * A constructor for the OrderList ADT, initializes
         * an empty OrderList.
         */
        OrderList() = default;

        /**
         * A constructor for the OrderList ADT, initializes
         * an order list with a single order.
         *
         * @param order an order.
         */
        explicit OrderList(Order &order) { order_list.push_back(order); }

        /**
         * Adds an order to the front of the order list if it is not
         * already in the list and it has the same price level.
         *
         * @param order an order, require that price associated with
         *              the order is equal to the price of the order
         *              list.
         */
        inline void addOrder(Order &order) { order_list.push_back(order); }

        /**
         * Removes the order associated with order_id
         * from the list - does not cancel the order.
         *
         * @param order_id the ID of the order to remove, require
         *                  that the order associated with ID
         *                  is in the order list.
         * @return the order that was removed from the list.
         * @throws Error if the order associated with order_id
         *               is not in the order list.
         */
        inline void removeOrder(const Order &order) {
            auto order_it = order_list.iterator_to(order);
            // Require that order is in list.
            assert(order_it != order_list.end());
            order_list.erase(order_it);
        }

        /**
         * @return the oldest order in the order list, require that the order
         *         list is non-empty.
         * @throws Error if the order list is empty.
         */
        inline Order front() {
            // Require that the order list is non-empty.
            assert(!order_list.empty());
            return order_list.front();
        }

        /**
         * Removes the oldest order from the order list, require that
         * the order list is non-empty.
         *
         * @throws Error if the order list is empty.
         */
        inline void popFront() {
            // Require that the order list is non-empty.
            assert(!order_list.empty());
            order_list.pop_front();
        }

        /**
         * @return the newest order in the order book, require that the
         *         order list is non-empty.
         * @throws Error if the order list is empty.
         */
        inline Order back() {
            // Require that the order list is non-empty.
            assert(!order_list.empty());
            return order_list.back();
        }

        /**
         * Removes the newest order from the order list, require
         * that the order list is non-empty.
         *
         * @throws Error if the order list is empty.
         */
        inline void popBack() {
            // Require that the order list is non-empty.
            assert(!order_list.empty());
            order_list.pop_back();
        }

        /**
         * Given an order, determines whether the order is in the order list.
         *
         * @param order an order.
         * @return true if the order is in the order list
         *         and false otherwise.
         */
        [[nodiscard]] inline bool hasOrder(const Order &order) const {
            return std::find(order_list.begin(),
                             order_list.end(), order) != order_list.end();
        }

        /**
         * @return true if the order list is empty and false otherwise.
         */
        [[nodiscard]] inline bool isEmpty() const { return order_list.empty(); }

        /**
         * @return the number of orders that are in the order list.
         */
        [[nodiscard]] inline size_t size() const { return order_list.size(); }

        /**
         * @return an iterator to the beginning of the order list.
         */
        inline auto begin() { return order_list.begin(); }

        /**
         * @return an iterator to the end of the order list.
         */
        inline auto end() { return order_list.end(); }

        /**
         * @return the string representation of the order list.
         */
        std::string toString() {
            std::string order_list_string;
            for (const auto &order: order_list) { order_list_string += order.toString(); }
            return order_list_string;
        }
    private:
        // A doubly-linked list of orders.
        list<OrderBook::Order> order_list;
    };
}
#endif //FAST_EXCHANGE_ORDER_LIST_H
