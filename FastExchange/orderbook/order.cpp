#include "order.h"

std::ostream& operator<<(std::ostream& os, const Order& order) {
    os << "Symbol ID: " << order.symbol_id << "\n";
    os << "Order ID: " << order.id << "\n";
    os << "User ID: " << order.user_id << "\n";
    os << "Order Action: " << order_action_to_string[static_cast<std::underlying_type<OrderStatus>::type>(order.action)]
       << "\n";
    os << "Order Side: " << order_side_to_string[static_cast<std::underlying_type<OrderStatus>::type>(order.side)]
       << "\n";
    os << "Order Type: " << order_type_to_string[static_cast<std::underlying_type<OrderStatus>::type>(order.type)]
       << "\n";
    os << "Order Price: " << order.price << "\n";
    os << "Order Quantity: " << order.quantity << "\n";
    os << "Order Quantity Executed: " << order.quantity_executed << "\n";
    return os;
}