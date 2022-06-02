#ifndef FAST_EXCHANGE_PRICE_LEVEL_H
#define FAST_EXCHANGE_PRICE_LEVEL_H
#include <list>
#include <unordered_map>
#include <algorithm>
#include "order.h"

using namespace boost::intrusive;

/**
 * A doubly linked list of orders that are associated with a single price.
 */
namespace OrderBook {
    class PriceLevel {
    public:
        // A doubly-linked list of orders.
        list<Order> order_list;
        // The total volume of the orders in the order list.
        Quantity volume = 0;

        /**
         * A constructor for the PriceLevel ADT, initializes
         * an empty PriceLevel.
         */
        PriceLevel() = default;

        /**
         * A constructor for the PriceLevel ADT, initializes
         * an order list with a single order.
         *
         * @param order an order.
         */
        explicit PriceLevel(Order &order) {
            order_list.push_back(order);
            volume += order.quantity;
        }

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
         * @return the string representation of the order list.
         */
        [[nodiscard]] std::string toString() const {
            std::string order_list_string;
            for (const auto &order: order_list) { order_list_string += order.toString(); }
            return order_list_string;
        }
    };
}
#endif //FAST_EXCHANGE_PRICE_LEVEL_H
