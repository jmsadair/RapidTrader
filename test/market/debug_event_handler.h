#ifndef RAPID_TRADER_DEBUG_EVENT_HANDLER_H
#define RAPID_TRADER_DEBUG_EVENT_HANDLER_H
#include "event_handler/event_handler.h"

class DebugEventHandler : public EventHandler
{
public:
    std::queue<OrderAdded> add_order_notifications;
    std::queue<OrderDeleted> delete_order_notifications;
    std::queue<ExecutedOrder> execute_order_notifications;
    std::queue<OrderUpdated> update_order_notifications;
    std::queue<SymbolAdded> add_symbol_notifications;
    std::queue<SymbolDeleted> delete_symbol_notifications;
    std::queue<OrderBookAdded> add_book_notifications;
    std::queue<OrderBookDeleted> delete_book_notifications;

    [[nodiscard]] bool empty() const
    {
        return add_order_notifications.empty() && delete_order_notifications.empty() && execute_order_notifications.empty() &&
               update_order_notifications.empty() && add_symbol_notifications.empty() && delete_symbol_notifications.empty() &&
               add_book_notifications.empty() && delete_book_notifications.empty();
    }

protected:
    void handleOrderAdded(const OrderAdded &notification) override
    {
        add_order_notifications.push(notification);
    }
    void handleOrderDeleted(const OrderDeleted &notification) override
    {
        delete_order_notifications.push(notification);
    }
    void handleOrderUpdated(const OrderUpdated &notification) override
    {
        update_order_notifications.push(notification);
    }
    void handleOrderExecuted(const ExecutedOrder &notification) override
    {
        execute_order_notifications.push(notification);
    }
    void handleSymbolAdded(const SymbolAdded &notification) override
    {
        add_symbol_notifications.push(notification);
    }
    void handleSymbolDeleted(const SymbolDeleted &notification) override
    {
        delete_symbol_notifications.push(notification);
    }
    void handleOrderBookAdded(const OrderBookAdded &notification) override
    {
        add_book_notifications.push(notification);
    }
    void handleOrderBookDeleted(const OrderBookDeleted &notification) override
    {
        delete_book_notifications.push(notification);
    }
};
#endif // RAPID_TRADER_DEBUG_EVENT_HANDLER_H
