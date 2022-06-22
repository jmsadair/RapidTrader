#ifndef FAST_EXCHANGE_PRICE_LEVEL_H
#define FAST_EXCHANGE_PRICE_LEVEL_H
#include <algorithm>
#include "order.h"

using namespace boost::intrusive;

namespace OrderBook {
/**
 * A doubly linked list of orders that are associated with a single price.
 */
class PriceLevel
{
public:
    // A doubly-linked list of orders.
    list<Order> orders;
    // The total volume of the orders in the order list.
    uint64_t volume = 0;

    /**
     * A constructor for the PriceLevel ADT, initializes
     * an empty PriceLevel.
     */
    PriceLevel() = default;
    PriceLevel(PriceLevel &&other) noexcept
        : volume(other.volume)
        , orders(std::move(other.orders))
    {}

    /**
     * A constructor for the PriceLevel ADT, initializes
     * an order list with a single order.
     *
     * @param order an order.
     */
    explicit PriceLevel(Order &order)
    {
        orders.push_back(order);
        volume += order.quantity;
    }

    /**
     * Given an order, determines whether the order is in the order list.
     *
     * @param order an order.
     * @return true if the order is in the order list
     *         and false otherwise.
     */
    [[nodiscard]] inline bool hasOrder(const Order &order) const
    {
        return std::find(orders.begin(), orders.end(), order) != orders.end();
    }
};
} // namespace OrderBook
#endif // FAST_EXCHANGE_PRICE_LEVEL_H
