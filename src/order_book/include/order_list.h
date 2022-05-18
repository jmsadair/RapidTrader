#ifndef FAST_EXCHANGE_ORDER_LIST_H
#define FAST_EXCHANGE_ORDER_LIST_H
#include <list>
#include <unordered_map>
#include "order.h"

using namespace boost::intrusive;

/**
 * A doubly linked list of orders that are associated with a single price.
 */
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
    explicit OrderList(Order& order);

    /**
     * Adds an order to the front of the order list if it is not
     * already in the list and it has the same price level.
     *
     * @param order an order, require that price associated with
     *              the order is equal to the price of the order
     *              list.
     * @throws Error if the order does not have the same price as
     *               the order list.
     */
    void addOrder(Order& order);

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
    void removeOrder(const Order& order);

    /**
     * Given an order ID, determines whether the order associated with that ID
     * is in the order list.
     *
     * @param order_id an ID associated with an order.
     * @return true if the order associated with order_id is in the order list
     *         and false otherwise.
     */
    bool hasOrder(const Order& order);

    /**
     * @returns true if the order list is empty and false otherwise.
     */
    bool isEmpty();

    /**
     * @returns the number of orders that are in the order list.
     */
    size_t size();

private:
    // A doubly-linked list of orders.
    list<Order> order_list;
};
#endif //FAST_EXCHANGE_ORDER_LIST_H
