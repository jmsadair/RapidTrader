#ifndef FAST_EXCHANGE_COMMAND_H
#define FAST_EXCHANGE_COMMAND_H
#include <iostream>
#include "order.h"

namespace Command {
struct Command
{
    virtual ~Command() = default;
};

/**
 * A message to place a new order.
 */
struct PlaceOrder : public Command
{
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
    PlaceOrder(uint64_t user_id_, uint64_t order_id_, uint32_t order_symbol_id_, uint32_t order_price_, uint64_t order_quantity_,
        OrderAction order_action_, OrderSide order_side_, OrderType order_type_);

    /**
     * Create an order from the command.
     *
     * @return
     */
    [[nodiscard]] inline Order makeOrder() const
    {
        return {order_action, order_side, order_type, order_quantity, order_price, order_id, user_id, order_symbol_id};
    }

    friend std::ostream &operator<<(std::ostream &os, const PlaceOrder &cmd);
};

/**
 * A message to cancel an existing order.
 */
struct CancelOrder : public Command
{
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
    CancelOrder(uint64_t user_id_, uint64_t order_id_, uint32_t order_symbol_id_);

    friend std::ostream &operator<<(std::ostream &os, const CancelOrder &cmd);
};

/**
 * A message to reduce an existing order.
 */
struct ReduceOrder : public Command
{
    const uint64_t order_id;
    const uint32_t order_symbol_id;
    const uint64_t quantity_to_reduce_by;

    /**
     * A constructor for the CancelOrder command.
     *
     * @param order_id_ the order ID associated with the order to be cancelled.
     * @param order_symbol_id_ the symbol ID associated with the order to be reduced.
     * @param quantity_to_reduce_by_ the quantity to reduce the order by, require that quantity_to_reduce_by_
     *                               is less than the current quantity of the order.
     */
    ReduceOrder(uint64_t order_id_, uint32_t order_symbol_id_, uint64_t quantity_to_reduce_by_);

    friend std::ostream &operator<<(std::ostream &os, const ReduceOrder &cmd);
};

/**
 * A message to create a new order book for the provided symbol.
 */
struct AddOrderBook : public Command
{
    const uint32_t orderbook_symbol_id;

    /**
     * A constructor for the AddOrderBook command.
     *
     * @param orderbook_symbol_id_ the symbol ID to create the order book for.
     */
    explicit AddOrderBook(uint32_t orderbook_symbol_id_);

    friend std::ostream &operator<<(std::ostream &os, const AddOrderBook &cmd);
};
} // namespace Command::Command
#endif // FAST_EXCHANGE_COMMAND_H
