#include "command.h"

Command::PlaceOrder::PlaceOrder(uint64_t user_id_, uint64_t order_id_, uint32_t order_symbol_id_, uint32_t order_price_,
    uint64_t order_quantity_, OrderAction order_action_, OrderSide order_side_, OrderType order_type_)
    : user_id(user_id_)
    , order_id(order_id_)
    , order_symbol_id(order_symbol_id_)
    , order_price(order_price_)
    , order_action(order_action_)
    , order_side(order_side_)
    , order_type(order_type_)
    , order_quantity(order_quantity_)
{}

Command::CancelOrder::CancelOrder(uint64_t user_id_, uint64_t order_id_, uint32_t order_symbol_id_)
    : user_id(user_id_)
    , order_id(order_id_)
    , order_symbol_id(order_symbol_id_)
{}

Command::ReduceOrder::ReduceOrder(uint64_t order_id_, uint32_t order_symbol_id_, uint64_t quantity_to_reduce_by_)
    : order_id(order_id_)
    , order_symbol_id(order_symbol_id_)
    , quantity_to_reduce_by(quantity_to_reduce_by_)
{}

Command::AddOrderBook::AddOrderBook(uint32_t orderbook_symbol_id_)
    : orderbook_symbol_id(orderbook_symbol_id_)
{}

std::ostream &operator<<(std::ostream &os, const Command::PlaceOrder &cmd)
{
    os << "User ID: " << cmd.user_id << "\n";
    os << "Order ID: " << cmd.order_id << "\n";
    os << "Order Symbol ID: " << cmd.order_symbol_id << "\n";
    os << "Order Action: " << order_action_to_string[static_cast<std::underlying_type<OrderAction>::type>(cmd.order_action)] << "\n";
    os << "Order Side: " << order_side_to_string[static_cast<std::underlying_type<OrderSide>::type>(cmd.order_side)] << "\n";
    os << "Order Type: " << order_type_to_string[static_cast<std::underlying_type<OrderType>::type>(cmd.order_type)] << "\n";
    os << "Order Price: " << cmd.order_price << "\n";
    os << "Order Quantity: " << cmd.order_quantity << "\n";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Command::CancelOrder &cmd)
{
    os << "User ID: " << cmd.user_id << "\n";
    os << "Order ID: " << cmd.order_id << "\n";
    os << "Order Symbol ID: " << cmd.order_symbol_id << "\n";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Command::ReduceOrder &cmd)
{
    os << "Order ID: " << cmd.order_id << "\n";
    os << "Order Symbol ID: " << cmd.order_symbol_id << "\n";
    os << "Quantity To Reduce Order By: " << cmd.quantity_to_reduce_by << "\n";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Command::AddOrderBook &cmd)
{
    os << "Symbol ID: " << cmd.orderbook_symbol_id << "\n";
    return os;
}