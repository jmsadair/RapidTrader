#include <iostream>
#include "order.h"

Order::Order(OrderType type_, OrderSide side_, OrderTimeInForce time_in_force_, uint32_t symbol_id_, uint64_t price_, uint64_t quantity_, uint64_t id_)
    : type(type_)
    , side(side_)
    , time_in_force(time_in_force_)
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
    os << "Type: " << order_type_to_string[static_cast<std::underlying_type<OrderType>::type>(order.type)] << "\n";
    os << "Side: " << order_side_to_string[static_cast<std::underlying_type<OrderSide>::type>(order.side)] << "\n";
    os << "TOF: " << order_tof_to_string[static_cast<std::underlying_type<OrderType>::type>(order.time_in_force)] << "\n";
    os << "Price: " << order.price << "\n";
    os << "Quantity: " << order.quantity << "\n";
    return os;
}
// LCOV_EXCL_STOP
