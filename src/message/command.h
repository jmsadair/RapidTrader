#ifndef FAST_EXCHANGE_COMMAND_H
#define FAST_EXCHANGE_COMMAND_H
#include "types.h"

namespace Message::Command {
    struct Command {
        virtual ~Command() = default;
    };

    /**
     * A message to place a new order.
     */
    struct PlaceOrder : public Command {
        PlaceOrder(UserID user_id_, OrderID order_id_, Symbol order_symbol_, Price order_price_, OrderAction order_action_,
                   OrderSide order_side_, OrderType order_type_, Quantity order_quantity_) :
                user_id(user_id_), order_id(order_id_), order_symbol(std::move(order_symbol_)), order_price(order_price_),
                order_action(order_action_), order_side(order_side_), order_type(order_type_),
                order_quantity(order_quantity_)
                {}

        const UserID user_id;
        const OrderID order_id;
        const Symbol order_symbol;
        const Price order_price;
        const OrderAction order_action;
        const OrderSide order_side;
        const OrderType order_type;
        Quantity order_quantity;

    };

    /**
     * A message to cancel an existing order.
     */
    struct CancelOrder : public Command {
        CancelOrder(UserID user_id_, OrderID order_id_, Symbol order_symbol_) :
                user_id(user_id_), order_id(order_id_), order_symbol(std::move(order_symbol_)) {}

        const UserID user_id;
        const OrderID order_id;
        const Symbol order_symbol;
    };

    /**
     * A message to create a new order book for the provided symbol.
     */
    struct AddOrderBook : public Command {
        explicit AddOrderBook(Symbol order_book_symbol_) :
                order_book_symbol(std::move(order_book_symbol_)) {}

        const Symbol order_book_symbol;
    };
}
#endif //FAST_EXCHANGE_COMMAND_H
