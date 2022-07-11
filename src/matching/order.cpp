#include <iostream>
#include "order.h"

Order::Order(OrderAction action_, OrderSide side_, OrderType type_, uint32_t symbol_id_, uint32_t price_, uint64_t quantity_, uint64_t id_)
    : action(action_)
    , side(side_)
    , type(type_)
    , symbol_id(symbol_id_)
    , price(price_)
    , quantity(quantity_)
    , id(id_)
{
    last_executed_price = 0;
    executed_quantity = 0;
    open_quantity = quantity;
    last_executed_price = 0;
    last_executed_quantity = 0;
}

// LCOV_EXCL_START
std::ostream &operator<<(std::ostream &os, const Order &order)
{
    os << "Symbol ID: " << order.symbol_id << "\n";
    os << "Order ID: " << order.id << "\n";
    os << "Order Action: " << order_action_to_string[static_cast<std::underlying_type<OrderAction>::type>(order.action)] << "\n";
    os << "Order Side: " << order_side_to_string[static_cast<std::underlying_type<OrderSide>::type>(order.side)] << "\n";
    os << "Order Type: " << order_type_to_string[static_cast<std::underlying_type<OrderType>::type>(order.type)] << "\n";
    os << "Order Price: " << order.price << "\n";
    os << "Order Quantity: " << order.quantity << "\n";
    return os;
}
// LCOV_EXCL_STOP
