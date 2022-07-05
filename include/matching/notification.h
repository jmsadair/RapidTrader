#ifndef RAPID_TRADER_NOTIFICATION_H
#define RAPID_TRADER_NOTIFICATION_H
#include "order.h"

struct Notification
{
    virtual ~Notification() = default;
};

struct MarketNotification : public Notification
{
    uint32_t symbol_id;
    explicit MarketNotification(uint32_t symbol_id_)
        : symbol_id(symbol_id_)
    {}
};

struct AddedSymbol : public MarketNotification
{
    std::string name;
    AddedSymbol(uint32_t symbol_id_, std::string name_)
        : MarketNotification(symbol_id_)
        , name(std::move(name_))
    {}
};

struct DeletedSymbol : public MarketNotification
{
    std::string name;
    DeletedSymbol(uint32_t symbol_id_, std::string name_)
        : MarketNotification(symbol_id_)
        , name(std::move(name_))
    {}
};

struct AddedOrderBook : public MarketNotification
{
    explicit AddedOrderBook(uint32_t symbol_id_)
        : MarketNotification(symbol_id_)
    {}
};

struct DeletedOrderBook : public MarketNotification
{
    explicit DeletedOrderBook(uint32_t symbol_id_)
        : MarketNotification(symbol_id_)
    {}
};

struct OrderNotification : public Notification
{
    Order order;
    explicit OrderNotification(Order order_)
        : order(std::move(order_))
    {}
};

struct AddedOrder : public OrderNotification
{
    explicit AddedOrder(Order order_)
        : OrderNotification(std::move(order_))
    {}
};

struct DeletedOrder : public OrderNotification
{
    explicit DeletedOrder(Order order_)
        : OrderNotification(std::move(order_))
    {}
};

struct ExecutedOrder : public OrderNotification
{
    explicit ExecutedOrder(Order order_)
        : OrderNotification(std::move(order_))
    {}
};

struct UpdatedOrder : public OrderNotification
{
    explicit UpdatedOrder(Order order_)
        : OrderNotification(std::move(order_))
    {}
};
#endif // RAPID_TRADER_NOTIFICATION_H
