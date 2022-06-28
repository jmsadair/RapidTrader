#include <iostream>
#include "event.h"

Event::TradeEvent::TradeEvent(uint64_t user_id_, uint64_t order_id_, uint64_t matched_order_id_, uint32_t order_price_,
    uint32_t matched_order_price_, uint64_t quantity_)
    : user_id(user_id_)
    , order_id(order_id_)
    , matched_order_id(matched_order_id_)
    , order_price(order_price_)
    , matched_order_price(matched_order_price_)
    , quantity(quantity_)
{}

Event::OrderExecuted::OrderExecuted(uint64_t user_id_, uint64_t order_id_, uint32_t order_price_, uint64_t order_quantity_)
    : user_id(user_id_)
    , order_id(order_id_)
    , order_price(order_price_)
    , order_quantity(order_quantity_)
{}

Event::RejectionEvent::RejectionEvent(uint64_t user_id_, uint64_t order_id_, uint32_t symbol_id_, uint32_t order_price_,
    uint64_t quantity_rejected_)
    : user_id(user_id_)
    , order_id(order_id_)
    , symbol_id(symbol_id_)
    , order_price(order_price_)
    , quantity_rejected(quantity_rejected_)
{}

std::ostream &operator<<(std::ostream &os, const Event::TradeEvent &event)
{
    os << "User ID: " << event.user_id << "\n";
    os << "Order ID: " << event.order_id << "\n";
    os << "Matched Order ID: " << event.matched_order_id << "\n";
    os << "Order Price: " << event.order_price << "\n";
    os << "Matched Order Price: " << event.matched_order_price << "\n";
    os << "Matched Quantity: " << event.quantity << "\n";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Event::OrderExecuted &event)
{
    os << "User ID: " << event.user_id << "\n";
    os << "Order ID: " << event.order_id << "\n";
    os << "Order Price: " << event.order_price << "\n";
    os << "Order Quantity: " << event.order_quantity << "\n";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Event::RejectionEvent &event)
{
    os << "User ID: " << event.user_id << "\n";
    os << "Order ID: " << event.order_id << "\n";
    os << "Order Symbol ID: " << event.symbol_id << "\n";
    os << "Order Price: " << event.order_price << "\n";
    os << "Order Quantity Rejected: " << event.quantity_rejected << "\n";
    return os;
}
