#ifndef FAST_EXCHANGE_MESSAGES_H
#define FAST_EXCHANGE_MESSAGES_H
#include "order.h"
struct PlaceOrder {
    UserID user_id;
    OrderID order_id;
    OrderAction order_action;
    OrderSide order_side;
    OrderType order_type;
};

struct CancelOrder {
    UserID user_id;
    OrderID order_id;
};

struct OrderExecuted {
    UserID user_id;
    OrderID order_id;
};

struct OrderCancelled {
    UserID user_id;
    OrderID order_id;
    Quantity remaining_quantity;
    OrderStatus order_status;
};
#endif //FAST_EXCHANGE_MESSAGES_H
