#ifndef FAST_EXCHANGE_EVENT_H
#define FAST_EXCHANGE_EVENT_H
#include "types.h"

namespace Message::Event {
    struct Event {
        virtual ~Event() = default;
    };

    struct TradeEvent : public Event {
        TradeEvent(UserID user_id_, OrderID order_id_, OrderID matched_order_id_, Price order_price_,
                   Price matched_order_price_, Quantity quantity_) :
                user_id(user_id_), order_id(order_id_), matched_order_id(matched_order_id_),
                order_price(order_price_), matched_order_price(matched_order_price_), quantity(quantity_)
        {}

        const UserID user_id;
        const OrderID order_id;
        const OrderID matched_order_id;
        const Price order_price;
        const Price matched_order_price;
        const Quantity quantity;
    };

    struct OrderExecuted : public Event {
        OrderExecuted(UserID user_id_, OrderID order_id_, Price order_price_, Quantity order_quantity_) :
                user_id(user_id_), order_id(order_id_), order_price(order_price_),
                order_quantity(order_quantity_)
        {}

        UserID user_id;
        OrderID order_id;
        Price order_price;
        Quantity order_quantity;
    };

    struct RejectionEvent : public Event {
        RejectionEvent(UserID user_id_, OrderID order_id_, Symbol symbol_, Price order_price_,
                       Quantity quantity_rejected_) :
                user_id(user_id_), order_id(order_id_), symbol(std::move(symbol_)), order_price(order_price_),
                quantity_rejected(quantity_rejected_), timestamp(std::chrono::system_clock::now()) {}

        const UserID user_id;
        const OrderID order_id;
        const Symbol symbol;
        const Price order_price;
        const Quantity quantity_rejected;
        const Time timestamp;
    };
}
#endif //FAST_EXCHANGE_EVENT_H
