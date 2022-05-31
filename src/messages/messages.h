#ifndef FAST_EXCHANGE_MESSAGES_H
#define FAST_EXCHANGE_MESSAGES_H
#include <utility>
#include "order.h"

using Symbol = std::string;

struct Command {
    virtual ~Command() = default;
};

struct PlaceOrderCommand : Command {
    PlaceOrderCommand(UserID user_id_, OrderID order_id_, Price order_price_, Symbol order_symbol_,
                        OrderAction order_action_, OrderSide order_side_, OrderType order_type_,
                        Quantity order_quantity_) :
                        user_id(user_id_), order_id(order_id_), order_price(order_price_),
                        order_symbol(std::move(order_symbol_)), order_action(order_action_), order_side(order_side_),
                        order_type(order_type_), order_quantity(order_quantity_)
    {}

    const UserID user_id;
    const OrderID order_id;
    const Price order_price;
    const Symbol order_symbol;
    const OrderAction order_action;
    const OrderSide order_side;
    const OrderType order_type;
    Quantity order_quantity;

};

struct CancelOrderCommand : Command {
    CancelOrderCommand(UserID user_id_, OrderID order_id_, Symbol order_symbol_) :
        user_id(user_id_), order_id(order_id_), order_symbol(std::move(order_symbol_))
    {}

    const UserID user_id;
    const OrderID order_id;
    const Symbol order_symbol;
};

struct AddOrderBookCommand : Command {
    explicit AddOrderBookCommand(Symbol order_book_symbol_) :
        order_book_symbol(std::move(order_book_symbol_))
    {}

    const Symbol order_book_symbol;
};

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
#endif //FAST_EXCHANGE_MESSAGES_H
