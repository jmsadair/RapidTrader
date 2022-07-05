#ifndef RAPID_TRADER_ORDERBOOK_H
#define RAPID_TRADER_ORDERBOOK_H
#include <exception>
#include <queue>
#include "notification.h"

class OrderBook
{
public:
    virtual void addOrder(const Order &order) = 0;
    virtual void deleteOrder(uint64_t order_id) = 0;
    [[nodiscard]] virtual bool canProcess(const Order &order) const = 0;
    virtual bool processMatching(Order &order, std::queue<Order> &processed_orders) = 0;
    virtual bool processMatching(std::queue<Order> &processed_orders) = 0;
    virtual bool executeOrder(uint64_t order_id, uint64_t quantity, uint32_t price) = 0;
    virtual bool executeOrder(uint64_t order_id, uint64_t quantity) = 0;
    virtual bool cancelOrder(uint64_t order_id, uint64_t quantity) = 0;
    [[nodiscard]] virtual bool empty() const = 0;
    [[nodiscard]] virtual uint32_t bestAsk() const = 0;
    [[nodiscard]] virtual uint32_t bestBid() const = 0;
    [[nodiscard]] virtual bool hasOrder(uint64_t order_id) const = 0;
    [[nodiscard]] virtual const Order &getOrder(uint64_t order_id) const = 0;
    virtual ~OrderBook() = default;
};
#endif // RAPID_TRADER_ORDERBOOK_H
