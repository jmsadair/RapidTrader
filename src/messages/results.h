#ifndef FAST_EXCHANGE_RESULTS_H
#define FAST_EXCHANGE_RESULTS_H
#include "order.h"

struct Result {
    virtual ~Result() = default;
};

struct OrderExecuted : Result {
    OrderExecuted(UserID user_id_, OrderID order_id_) :
            user_id(user_id_), order_id(order_id_)
    {}

    const UserID user_id;
    const OrderID order_id;
};

struct OrderCancelled : Result {
    OrderCancelled(UserID user_id_, OrderID order_id_, Quantity remaining_quantity_, OrderStatus order_status_) :
            user_id(user_id_), order_id(order_id_), remaining_quantity(remaining_quantity_), order_status(order_status_)
    {}

    const UserID user_id;
    const OrderID order_id;
    const Quantity remaining_quantity;
    const OrderStatus order_status;
};
#endif //FAST_EXCHANGE_RESULTS_H
