#include <iostream>
#include "notification.h"

// LCOV_EXCL_START
std::ostream &operator<<(std::ostream &os, const AddedSymbol &notification)
{
    os << "ADDED SYMBOL: " << notification.name << " " << notification.symbol_id << "\n";
    return os;
}
std::ostream &operator<<(std::ostream &os, const DeletedSymbol &notification)
{
    os << "DELETED SYMBOL: " << notification.name << " " << notification.symbol_id << "\n";
    return os;
}
std::ostream &operator<<(std::ostream &os, const AddedOrderBook &notification)
{
    os << "ADDED ORDERBOOK: " << notification.symbol_id << "\n";
    return os;
}
std::ostream &operator<<(std::ostream &os, const DeletedOrderBook &notification)
{
    os << "DELETED ORDERBOOK: " << notification.symbol_id << "\n";
    return os;
}
std::ostream &operator<<(std::ostream &os, const AddedOrder &notification)
{
    os << "ADDED ORDER: " << notification.order.getOrderID() << "\n";
    return os;
}
std::ostream &operator<<(std::ostream &os, const ExecutedOrder &notification)
{
    os << "EXECUTED ORDER: " << notification.order.getOrderID() << "\n";
    return os;
}
std::ostream &operator<<(std::ostream &os, const DeletedOrder &notification)
{
    os << "DELETED ORDER: " << notification.order.getOrderID() << "\n";
    return os;
}
std::ostream &operator<<(std::ostream &os, const UpdatedOrder &notification)
{
    os << "UPDATED ORDER: " << notification.order.getOrderID() << "\n";
    return os;
}
// LCOV_EXCL_STOP