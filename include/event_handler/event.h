#ifndef RAPID_TRADER_EVENT_H
#define RAPID_TRADER_EVENT_H
#include "order.h"

enum class Error
{
    DuplicateOrder = 0,
    SymbolDoesNotExist = 1,
    OrderDoesNotExist = 2,
    InvalidQuantity = 3,
    InvalidPrice = 4,
    InvalidOrderID = 6,
    DuplicateSymbol = 7
};

struct Event
{
    virtual ~Event() = default;
};

struct OperationRejected : public Event
{
    uint64_t id;
    Error error;
    OperationRejected(uint64_t id_, Error error_)
        : id(id_)
        , error(error_)
    {}
};

struct MarketEvent : public Event
{
    uint32_t symbol_id;
    explicit MarketEvent(uint32_t symbol_id_)
        : symbol_id(symbol_id_)
    {}
};

struct SymbolAdded : public MarketEvent
{
    std::string name;
    SymbolAdded(uint32_t symbol_id_, std::string name_)
        : MarketEvent(symbol_id_)
        , name(std::move(name_))
    {}

    friend std::ostream &operator<<(std::ostream &os, const SymbolAdded &notification);
};

struct OrderEvent : public Event
{
    Order order;
    explicit OrderEvent(Order order_)
        : order(std::move(order_))
    {}
};

struct OrderAdded : public OrderEvent
{
    explicit OrderAdded(Order order_)
        : OrderEvent(std::move(order_))
    {}

    friend std::ostream &operator<<(std::ostream &os, const OrderAdded &notification);
};

struct OrderDeleted : public OrderEvent
{
    explicit OrderDeleted(Order order_)
        : OrderEvent(std::move(order_))
    {}

    friend std::ostream &operator<<(std::ostream &os, const OrderDeleted &notification);
};

struct ExecutedOrder : public OrderEvent
{
    explicit ExecutedOrder(Order order_)
        : OrderEvent(std::move(order_))
    {}

    friend std::ostream &operator<<(std::ostream &os, const ExecutedOrder &notification);
};

struct OrderUpdated : public OrderEvent
{
    explicit OrderUpdated(Order order_)
        : OrderEvent(std::move(order_))
    {}

    friend std::ostream &operator<<(std::ostream &os, const OrderUpdated &notification);
};
#endif // RAPID_TRADER_EVENT_H
