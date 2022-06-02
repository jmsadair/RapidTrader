#ifndef FAST_EXCHANGE_RESULTS_H
#define FAST_EXCHANGE_RESULTS_H
#include "order.h"
#include <iostream>
struct Result {
    virtual ~Result() = default;
};

struct OrderExecuted : public Result {
    OrderExecuted(UserID user_id_, OrderID order_id_) :
            user_id(user_id_), order_id(order_id_)
    {}

    const UserID user_id;
    const OrderID order_id;
};

struct OrderCancelled : public Result {
    OrderCancelled(UserID user_id_, OrderID order_id_, Quantity remaining_quantity_, OrderStatus order_status_) :
            user_id(user_id_), order_id(order_id_), remaining_quantity(remaining_quantity_), order_status(order_status_)
    {}

    const UserID user_id;
    const OrderID order_id;
    const Quantity remaining_quantity;
    const OrderStatus order_status;
};

struct OrderAddedToBook : public Result {
    OrderAddedToBook(UserID user_id_, OrderID order_id_, Quantity order_quantity_, OrderStatus order_status_) :
        user_id(user_id_), order_id(order_id_), order_quantity(order_quantity_), order_status(order_status_)
    {}

    const UserID user_id;
    const OrderID order_id;
    const Quantity order_quantity;
    const OrderStatus order_status;
};
#endif //FAST_EXCHANGE_RESULTS_H
