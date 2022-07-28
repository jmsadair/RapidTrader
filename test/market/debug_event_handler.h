#ifndef RAPID_TRADER_DEBUG_EVENT_HANDLER_H
#define RAPID_TRADER_DEBUG_EVENT_HANDLER_H
#include "event_handler/event_handler.h"

class DebugEventHandler : public EventHandler
{
public:
    std::queue<OrderAdded> add_order_events;
    std::queue<OrderDeleted> delete_order_events;
    std::queue<ExecutedOrder> execute_order_events;
    std::queue<OrderUpdated> update_order_events;
    std::queue<SymbolAdded> add_symbol_events;

    [[nodiscard]] bool empty() const
    {
        return add_order_events.empty() && delete_order_events.empty() && execute_order_events.empty() && update_order_events.empty() &&
               add_symbol_events.empty();
    }

protected:
    void handleOrderAdded(const OrderAdded &notification) override
    {
        add_order_events.push(notification);
    }
    void handleOrderDeleted(const OrderDeleted &notification) override
    {
        delete_order_events.push(notification);
    }
    void handleOrderUpdated(const OrderUpdated &notification) override
    {
        update_order_events.push(notification);
    }
    void handleOrderExecuted(const ExecutedOrder &notification) override
    {
        execute_order_events.push(notification);
    }
    void handleSymbolAdded(const SymbolAdded &notification) override
    {
        add_symbol_events.push(notification);
    }
};
#endif // RAPID_TRADER_DEBUG_EVENT_HANDLER_H
