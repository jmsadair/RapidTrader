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

        /**
         * A constructor for the PlaceOrder command.
         *
         * @param user_id_ the ID of the user placing the order.
         * @param order_id_ the ID that will be associated with the order.
         * @param order_symbol_id_ the ID of the symbol that the order is associated with.
         * @param order_price_ the price that the order will have.
         * @param order_quantity_ the quantity that the order will have.
         * @param order_action_ the action that the order will have - market or limit.
         * @param order_side_ the side the order will be on - ask or bid.
         * @param order_type_ the type that the order will have - GTC, IOC, or FOK.
         */
        inline PlaceOrder(uint64_t user_id_, uint64_t order_id_, uint32_t order_symbol_id_, uint32_t order_price_,
                   uint64_t order_quantity_, OrderAction order_action_, OrderSide order_side_, OrderType order_type_) :
                user_id(user_id_), order_id(order_id_), order_symbol_id(order_symbol_id_), order_price(order_price_),
                order_action(order_action_), order_side(order_side_), order_type(order_type_),
                order_quantity(order_quantity_)
        {}

        /**
         * Create an order from the command.
         *
         * @return
         */
        [[nodiscard]] inline Order makeOrder() const {
            return {order_action, order_side, order_type, order_quantity, order_price, order_id, user_id,
                    order_symbol_id};
        }

        friend std::ostream& operator<<(std::ostream& os, const PlaceOrder& cmd) {
            os << "User ID: " << cmd.user_id << "\n";
            os << "Order ID: " << cmd.order_id << "\n";
            os << "Order Symbol ID: " << cmd.order_symbol_id << "\n";
            os << "Order Action: " <<
               order_action_to_string[static_cast<std::underlying_type<OrderAction>::type>(cmd.order_action)] << "\n";
            os << "Order Side: " <<
                order_side_to_string[static_cast<std::underlying_type<OrderSide>::type>(cmd.order_side)] << "\n";
            os << "Order Type: " <<
                order_type_to_string[static_cast<std::underlying_type<OrderType>::type>(cmd.order_type)] << "\n";
            os << "Order Price: " << cmd.order_price << "\n";
            os << "Order Quantity: " << cmd.order_quantity << "\n";
            return os;
        }
    };

    /**
     * A message to cancel an existing order.
     */
    struct CancelOrder : public Command {
        const uint64_t user_id;
        const uint64_t order_id;
        const uint32_t order_symbol_id;

        /**
         * A constructor for the CancelOrder command.
         *
         * @param user_id_ the user ID associated with the order to be cancelled.
         * @param order_id_ the order ID associated with the order to be cancelled.
         * @param order_symbol_id_ the symbol ID associated with the order to be cancelled.
         */
        inline CancelOrder(uint64_t user_id_, uint64_t order_id_, uint32_t order_symbol_id_) :
                user_id(user_id_), order_id(order_id_), order_symbol_id(order_symbol_id_)
        {}

        friend std::ostream& operator<<(std::ostream& os, const CancelOrder& cmd) {
            os << "User ID: " << cmd.user_id << "\n";
            os << "Order ID: " << cmd.order_id << "\n";
            os << "Order Symbol ID: " << cmd.order_symbol_id << "\n";
            return os;
        }
    };

    /**
     * A message to create a new order book for the provided symbol.
     */
    struct AddOrderBook : public Command {
        const uint32_t orderbook_symbol_id;

        /**
         * A constructor for the AddOrderBook command.
         *
         * @param orderbook_symbol_id_ the symbol ID to create the order book for.
         */
        inline explicit AddOrderBook(uint32_t orderbook_symbol_id_) : orderbook_symbol_id(orderbook_symbol_id_) {}

        friend std::ostream& operator<<(std::ostream& os, const AddOrderBook& cmd) {
            os << "Symbol ID: " << cmd.orderbook_symbol_id << "\n";
            return os;
        }
    };
}
#endif //FAST_EXCHANGE_COMMAND_H
