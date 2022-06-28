#ifndef FAST_EXCHANGE_EVENT_H
#define FAST_EXCHANGE_EVENT_H
#include "types.h"

namespace Event {
struct Event
{
    virtual ~Event() = default;
};

/**
 * An immutable message indicating that an order has traded.
 */
struct TradeEvent : public Event
{
    const uint64_t user_id;
    const uint64_t order_id;
    const uint64_t matched_order_id;
    const uint32_t order_price;
    const uint32_t matched_order_price;
    const uint64_t quantity;

    /**
     * A constructor for the TradeEvent event.
     *
     * @param user_id_ the user ID associated with the order that was just traded.
     * @param order_id_ the order ID associated with the order that was just traded.
     * @param matched_order_id_ the order ID of the other order that was traded.
     * @param order_price_ the price of the order being traded.
     * @param matched_order_price_ the price at which both orders were traded.
     * @param quantity_ the quantity of the the orders that were traded.
     */
    TradeEvent(uint64_t user_id_, uint64_t order_id_, uint64_t matched_order_id_, uint32_t order_price_, uint32_t matched_order_price_,
        uint64_t quantity_);

    friend std::ostream &operator<<(std::ostream &os, const TradeEvent &event);
};

/**
 * A immutable message to indicate that an add order has been executed.
 * That is, the order has been completely filled.
 */
struct OrderExecuted : public Event
{
    const uint64_t user_id;
    const uint64_t order_id;
    const uint32_t order_price;
    const uint64_t order_quantity;

    /**
     * A constructor for the OrderExecuted event.
     *
     * @param user_id_ the user ID associated with the executed order.
     * @param order_id_ the ID of the executed order.
     * @param order_price_ the price of the executed order.
     * @param order_quantity_ the quantity of the executed order.
     */
    OrderExecuted(uint64_t user_id_, uint64_t order_id_, uint32_t order_price_, uint64_t order_quantity_);

    friend std::ostream &operator<<(std::ostream &os, const OrderExecuted &event);
};

/**
 * An immutable message to indicate that an order has been rejected.
 */
struct RejectionEvent : public Event
{
    const uint64_t user_id;
    const uint64_t order_id;
    const uint32_t symbol_id;
    const uint32_t order_price;
    const uint64_t quantity_rejected;

    /**
     * A constructor for  RejectionEvent event.
     *
     * @param user_id_ the user ID associated with the order that was rejected.
     * @param order_id_ the ID associated with the order that was rejected.
     * @param symbol_id_ the symbol ID associated with the order that was rejected.
     * @param order_price_ the price of the order that was rejected.
     * @param quantity_rejected_ the quantity of the order that was not executed.
     */
    RejectionEvent(uint64_t user_id_, uint64_t order_id_, uint32_t symbol_id_, uint32_t order_price_, uint64_t quantity_rejected_);

    friend std::ostream &operator<<(std::ostream &os, const RejectionEvent &event);
};
} // namespace Command::Event
#endif // FAST_EXCHANGE_EVENT_H