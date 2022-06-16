#ifndef FAST_EXCHANGE_PRICE_LEVEL_H
#define FAST_EXCHANGE_PRICE_LEVEL_H
#include <algorithm>
#include "order.h"

using namespace boost::intrusive;

namespace OrderBook {
    /**
     * A doubly linked list of orders that are associated with a single price.
     *
     * Representation Invariants:
     * 1. All orders in the order list must have the same price.
     * 2. All orders in the order list must be on the same side - all orders are ask or all are bid.
     * 3. The sum of the quantities of the orders in the list must be equal to volume.
     * 4. The orders in the list must be GTC orders, there cannot be any FOK or IOC orders in the order list.
     */
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

        /**
         * Verifies that the price level satisfies its representation invariants.
         *
         * @param expected_price the expected price of all orders in the price level.
         * @param expected_order_side the expected side of all orders in the price level.
         * @throws Error if not all orders have the expected price, if not all orders are on
         *               the expected side, or the sum of the quantities of the orders in the
         *               price level is not equal to the price level volume.
         */
        void checkRep(uint32_t expected_price, OrderSide expected_order_side) {
            uint64_t quantity_sum = 0;
            for (const auto& order : order_list) {
                // Check that all orders have the expected price.
                assert(order.price == expected_price);
                // Check that all orders are GTC (should not be any FOK or IOC orders).
                assert(order.type == OrderType::GoodTillCancel);
                // Check that all orders are on the expected side.
                assert(order.side == expected_order_side);
                // Sum the order quantities.
                quantity_sum += order.executableQuantity();
            }
            // Check that the sum of order quantities is the same as the price level volume.
            assert(quantity_sum == volume);
        }
    };
}
#endif //FAST_EXCHANGE_PRICE_LEVEL_H
