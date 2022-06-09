#ifndef FAST_EXCHANGE_PRICE_LEVEL_H
#define FAST_EXCHANGE_PRICE_LEVEL_H
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
        uint64_t volume = 0;

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
    };
}
#endif //FAST_EXCHANGE_PRICE_LEVEL_H
