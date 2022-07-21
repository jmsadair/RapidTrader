#include <iostream>
#include "order.h"

Order::Order(OrderType type_, OrderSide side_, OrderTimeInForce time_in_force_, uint32_t symbol_id_, uint64_t price_, uint64_t stop_price_,
    uint64_t trail_amount_, uint64_t quantity_, uint64_t id_)
    : type(type_)
    , side(side_)
    , time_in_force(time_in_force_)
    , symbol_id(symbol_id_)
    , price(price_)
    , stop_price(stop_price_)
    , trail_amount(trail_amount_)
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

Order Order::marketAskOrder(uint64_t order_id, uint32_t symbol_id, uint64_t quantity, OrderTimeInForce time_in_force)
{
    assert(time_in_force != OrderTimeInForce::GTC && "Market orders cannot gave GTC time in force!");
    assert(order_id > 0 && "Order ID must be positive!");
    assert(symbol_id > 0 && "Symbol ID must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    return Order{OrderType::Market, OrderSide::Ask, time_in_force, symbol_id, 0, 0, 0, quantity, order_id};
}

Order Order::marketBidOrder(uint64_t order_id, uint32_t symbol_id, uint64_t quantity, OrderTimeInForce time_in_force)
{
    assert(time_in_force != OrderTimeInForce::GTC && "Market orders cannot gave GTC time in force!");
    assert(order_id > 0 && "Order ID must be positive!");
    assert(symbol_id > 0 && "Symbol ID must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    return Order{OrderType::Market, OrderSide::Bid, time_in_force, symbol_id, 0, 0, 0, quantity, order_id};
}

Order Order::limitAskOrder(uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t quantity, OrderTimeInForce time_in_force)
{
    assert(order_id > 0 && "Order ID must be positive!");
    assert(symbol_id > 0 && "Symbol ID must be positive!");
    assert(price > 0 && "Price must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    return Order{OrderType::Limit, OrderSide::Ask, time_in_force, symbol_id, price, 0, 0, quantity, order_id};
}

Order Order::limitBidOrder(uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t quantity, OrderTimeInForce time_in_force)
{
    assert(order_id > 0 && "Order ID must be positive!");
    assert(symbol_id > 0 && "Symbol ID must be positive!");
    assert(price > 0 && "Price must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    return Order{OrderType::Limit, OrderSide::Bid, time_in_force, symbol_id, price, 0, 0, quantity, order_id};
}

Order Order::stopAskOrder(uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force)
{
    assert(time_in_force != OrderTimeInForce::GTC && "Stop orders cannot gave GTC time in force!");
    assert(order_id > 0 && "Order ID must be positive!");
    assert(symbol_id > 0 && "Symbol ID must be positive!");
    assert(stop_price > 0 && "Price must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    return Order{OrderType::Stop, OrderSide::Ask, time_in_force, symbol_id, 0, stop_price, 0, quantity, order_id};
}

Order Order::stopBidOrder(uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force)
{
    assert(time_in_force != OrderTimeInForce::GTC && "Stop orders cannot gave GTC time in force!");
    assert(order_id > 0 && "Order ID must be positive!");
    assert(symbol_id > 0 && "Symbol ID must be positive!");
    assert(stop_price > 0 && "Stop Price must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    return Order{OrderType::Stop, OrderSide::Bid, time_in_force, symbol_id, 0, stop_price, 0, quantity, order_id};
}

Order Order::stopLimitAskOrder(
    uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t stop_price_, uint64_t quantity, OrderTimeInForce time_in_force)
{
    assert(order_id > 0 && "Order ID must be positive!");
    assert(symbol_id > 0 && "Symbol ID must be positive!");
    assert(price > 0 && "Price must be positive!");
    assert(stop_price_ > 0 && "Stop Price must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    return Order{OrderType::StopLimit, OrderSide::Ask, time_in_force, symbol_id, price, stop_price_, 0, quantity, order_id};
}

Order Order::stopLimitBidOrder(
    uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force)
{
    assert(order_id > 0 && "Order ID must be positive!");
    assert(symbol_id > 0 && "Symbol ID must be positive!");
    assert(price > 0 && "Price must be positive!");
    assert(stop_price > 0 && "Stop Price must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    return Order{OrderType::StopLimit, OrderSide::Bid, time_in_force, symbol_id, price, stop_price, 0, quantity, order_id};
}

Order Order::trailingStopAskOrder(
    uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t trail_amount, uint64_t quantity, OrderTimeInForce time_in_force)
{
    assert(time_in_force != OrderTimeInForce::GTC && "Stop orders cannot gave GTC time in force!");
    assert(order_id > 0 && "Order ID must be positive!");
    assert(symbol_id > 0 && "Symbol ID must be positive!");
    assert(stop_price > 0 && "Stop Price must be positive!");
    assert(trail_amount > 0 && "Stop Price must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    return Order{OrderType::TrailingStop, OrderSide::Ask, time_in_force, symbol_id, 0, stop_price, trail_amount, quantity, order_id};
}

Order Order::trailingStopBidOrder(
    uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t trail_amount, uint64_t quantity, OrderTimeInForce time_in_force)
{
    assert(time_in_force != OrderTimeInForce::GTC && "Stop orders cannot gave GTC time in force!");
    assert(order_id > 0 && "Order ID must be positive!");
    assert(symbol_id > 0 && "Symbol ID must be positive!");
    assert(stop_price > 0 && "Stop Price must be positive!");
    assert(trail_amount > 0 && "Stop Price must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    return Order{OrderType::TrailingStop, OrderSide::Bid, time_in_force, symbol_id, 0, stop_price, trail_amount, quantity, order_id};
}

Order Order::trailingStopLimitAskOrder(uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t stop_price, uint64_t trail_amount,
    uint64_t quantity, OrderTimeInForce time_in_force)
{
    assert(time_in_force != OrderTimeInForce::GTC && "Stop orders cannot gave GTC time in force!");
    assert(order_id > 0 && "Order ID must be positive!");
    assert(symbol_id > 0 && "Symbol ID must be positive!");
    assert(price > 0 && "Price must be positive!");
    assert(stop_price > 0 && "Stop Price must be positive!");
    assert(trail_amount > 0 && "Trail amount must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    return Order{
        OrderType::TrailingStopLimit, OrderSide::Ask, time_in_force, symbol_id, price, stop_price, trail_amount, quantity, order_id};
}

Order Order::trailingStopLimitBidOrder(uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t stop_price, uint64_t trail_amount,
    uint64_t quantity, OrderTimeInForce time_in_force)
{
    assert(time_in_force != OrderTimeInForce::GTC && "Stop orders cannot gave GTC time in force!");
    assert(order_id > 0 && "Order ID must be positive!");
    assert(symbol_id > 0 && "Symbol ID must be positive!");
    assert(price > 0 && "Price must be positive!");
    assert(stop_price > 0 && "Stop Price must be positive!");
    assert(trail_amount > 0 && "Trail amount must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    return Order{
        OrderType::TrailingStopLimit, OrderSide::Bid, time_in_force, symbol_id, price, stop_price, trail_amount, quantity, order_id};
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
    // Price should always be positive for limit and stop limit orders.
    if (type == OrderType::Limit || type == OrderType::StopLimit || type == OrderType::TrailingStopLimit)
        assert(price > 0 && "Limit, stop limit, and trailing stop limit orders must have a positive price!");
    // Market orders and stop  orders can only have time in force FOK or IOC.
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
