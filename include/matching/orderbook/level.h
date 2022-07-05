#ifndef RAPID_TRADER_LEVEL_H
#define RAPID_TRADER_LEVEL_H
#include <algorithm>
#include "order.h"

using namespace boost::intrusive;

class Level
{
public:
    explicit Level(uint32_t price_)
        : price(price_)
    {
        volume = 0;
    }
    [[nodiscard]] inline uint32_t getPrice() const
    {
        return price;
    }
    [[nodiscard]] inline uint64_t getVolume() const
    {
        return volume;
    }
    inline size_t size()
    {
        return orders.size();
    }
    inline bool empty()
    {
        return orders.empty();
    }
    inline Order &front()
    {
        return orders.front();
    }
    inline Order &back()
    {
        return orders.back();
    }
    inline void addOrder(Order &order)
    {
        volume += order.getOpenQuantity();
        orders.push_front(order);
    };
    inline void popFront()
    {
        Order &order_to_remove = orders.front();
        volume -= order_to_remove.getOpenQuantity();
        orders.pop_front();
    };
    inline void popBack()
    {
        Order &order_to_remove = orders.back();
        volume -= order_to_remove.getOpenQuantity();
        orders.pop_back();
    }
    inline void deleteOrder(const Order &order)
    {
        volume -= order.getOpenQuantity();
        orders.remove(order);
    }

private:
    // A doubly-linked list of orders.
    list<Order> orders;
    // The total volume of the orders in the order list.
    uint64_t volume;
    // The price of the level.
    uint32_t price;
};
#endif // RAPID_TRADER_LEVEL_H
