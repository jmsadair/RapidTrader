#include <iostream>
#include "order.h"

Order::Order(OrderType type_, OrderSide side_, OrderTimeInForce time_in_force_, uint32_t symbol_id_, uint64_t price_, uint64_t stop_price_,
    uint64_t quantity_, uint64_t id_)
    : type(type_)
    , side(side_)
    , time_in_force(time_in_force_)
    , symbol_id(symbol_id_)
    , price(price_)
    , stop_price(stop_price_)
    , quantity(quantity_)
    , id(id_)
{
    last_executed_price = 0;
    executed_quantity = 0;
    open_quantity = quantity;
    last_executed_price = 0;
    last_executed_quantity = 0;
    ORDER_CHECK_INVARIANTS;
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

void Order::checkInvariants() const
{
    // Stop order price should always be positive if the order is a stop order.
    // Price should always be positive for limit and stop limit orders.
    if (type == OrderType::Limit)
        assert(price > 0 && "Limit orders must have a positive price!");
    if (type == OrderType::Stop || type == OrderType::TrailingStop)
        assert(stop_price > 0 && "Stop and trailing stop orders must have a positive stop price!");
    if (type == OrderType::StopLimit || type == OrderType::TrailingStopLimit)
        assert(stop_price > 0 && "Stop limit and trailing stop limit orders must have a positive stop price!");
    // Market orders and stop orders can only have time in force FOK or IOC.
    if (type == OrderType::Market || type == OrderType::Stop || type == OrderType::TrailingStop)
        assert(time_in_force != OrderTimeInForce::GTC && "Market and stop orders cannot have GTC time in force!");
    // Quantity should always be positive
    assert(quantity > 0 && "Orders must have a positive quantity!");
    // Last executed quantity and executed quantity should never exceed the quantity of the order.
    assert(last_executed_quantity <= quantity && "Last executed quantity of the order should never exceed the quantity of the order!");
    assert(executed_quantity <= quantity && "Executed quantity of the order should never exceed the quantity of the order!");
    // Order ID should never be 0.
    assert(id > 0 && "Order ID must be positive!");
}
// LCOV_EXCL_STOP
