#ifndef FAST_EXCHANGE_COMMAND_H
#define FAST_EXCHANGE_COMMAND_H
#include "order.h"

namespace Message::Command {
    struct Command {
        virtual ~Command() = default;
    };

    /**
     * A message to place a new order.
     */
    struct PlaceOrder : public Command {
        const uint64_t user_id;
        const uint64_t order_id;
        const uint32_t order_symbol_id;
        const uint32_t order_price;
        const uint64_t order_quantity;
        const OrderAction order_action;
        const OrderSide order_side;
        const OrderType order_type;

        inline PlaceOrder(uint64_t user_id_, uint64_t order_id_, uint32_t order_symbol_id_, uint32_t order_price_,
                   uint64_t order_quantity_, OrderAction order_action_, OrderSide order_side_, OrderType order_type_) :
                user_id(user_id_), order_id(order_id_), order_symbol_id(order_symbol_id_), order_price(order_price_),
                order_action(order_action_), order_side(order_side_), order_type(order_type_),
                order_quantity(order_quantity_)
        {}

        [[nodiscard]] inline Order makeOrder() const {
            return {order_action, order_side, order_type, order_quantity, order_price, order_id, user_id,
                    order_symbol_id};
        }

    };

    /**
     * A message to cancel an existing order.
     */
    struct CancelOrder : public Command {
        const uint64_t user_id;
        const uint64_t order_id;
        const uint32_t order_symbol_id;

        inline CancelOrder(uint64_t user_id_, uint64_t order_id_, uint32_t order_symbol_id_) :
                user_id(user_id_), order_id(order_id_), order_symbol_id(order_symbol_id_)
        {}
    };

    /**
     * A message to create a new order book for the provided symbol.
     */
    struct AddOrderBook : public Command {
        const uint32_t orderbook_symbol_id;

        inline explicit AddOrderBook(uint32_t orderbook_symbol_id_) : orderbook_symbol_id(orderbook_symbol_id_) {}
    };
}
#endif //FAST_EXCHANGE_COMMAND_H
