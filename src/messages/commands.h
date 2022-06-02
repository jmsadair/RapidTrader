#ifndef FAST_EXCHANGE_COMMANDS_H
#define FAST_EXCHANGE_COMMANDS_H
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
    const Quantity order_quantity;

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
#endif //FAST_EXCHANGE_COMMANDS_H
