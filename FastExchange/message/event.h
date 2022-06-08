#ifndef FAST_EXCHANGE_EVENT_H
#define FAST_EXCHANGE_EVENT_H
#include "types.h"

namespace Message::Event {
    struct Event {
        virtual ~Event() = default;
    };

    /**
     * A message indicating that an order has traded.
     */
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

    /**
     * A message to indicate that an add order has been executed. That is, the order has been completely filled.
     */
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

    /**
     * A message to indicate that an order has been rejected.
     */
    struct RejectionEvent : public Event {
        RejectionEvent(uint64_t user_id_, uint64_t order_id_, uint32_t symbol_id_, uint32_t order_price_,
                       uint64_t quantity_rejected_) :
                user_id(user_id_), order_id(order_id_), symbol_id(symbol_id_), order_price(order_price_),
                quantity_rejected(quantity_rejected_)
        {}

        const uint64_t user_id;
        const uint64_t order_id;
        const uint32_t symbol_id;
        const uint32_t order_price;
        const uint64_t quantity_rejected;
    };
}
#endif //FAST_EXCHANGE_EVENT_H
