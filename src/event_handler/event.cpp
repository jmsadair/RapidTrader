#include <iostream>
#include "event_handler/event.h"

// LCOV_EXCL_START
std::ostream &operator<<(std::ostream &os, const SymbolAdded &notification)
{
    os << "ADDED SYMBOL: " << notification.name << " " << notification.symbol_id << "\n";
    return os;
}

std::ostream &operator<<(std::ostream &os, const SymbolDeleted &notification)
{
    os << "DELETED SYMBOL: " << notification.name << " " << notification.symbol_id << "\n";
    return os;
}

std::ostream &operator<<(std::ostream &os, const OrderAdded &notification)
{
    os << "ADDED ORDER: " << notification.order.getOrderID() << "\n";
    return os;
}
std::ostream &operator<<(std::ostream &os, const ExecutedOrder &notification)
{
    os << "EXECUTED ORDER: " << notification.order.getOrderID() << "\n";
    return os;
}
std::ostream &operator<<(std::ostream &os, const OrderDeleted &notification)
{
    os << "DELETED ORDER: " << notification.order.getOrderID() << "\n";
    return os;
}
std::ostream &operator<<(std::ostream &os, const OrderUpdated &notification)
{
    os << "UPDATED ORDER: " << notification.order.getOrderID() << "\n";
    return os;
}
// LCOV_EXCL_STOP

