#ifndef RAPID_TRADER_DEBUG_NOTIFICATION_PROCESSOR_H
#define RAPID_TRADER_DEBUG_NOTIFICATION_PROCESSOR_H
#include "notification_processor.h"

class DebugNotificationProcessor : public NotificationProcessor
{
public:
    std::queue<AddedSymbol> add_symbol_notifications;
    std::queue<DeletedSymbol> delete_symbol_notifications;
    std::queue<AddedOrderBook> add_book_notifications;
    std::queue<DeletedOrderBook> delete_book_notifications;
    std::queue<AddedOrder> add_order_notifications;
    std::queue<DeletedOrder> delete_order_notifications;
    std::queue<ExecutedOrder> execute_order_notifications;
    std::queue<UpdatedOrder> update_order_notifications;

    [[nodiscard]] bool empty() const
    {
        return add_symbol_notifications.empty() && delete_symbol_notifications.empty() && add_book_notifications.empty() &&
               delete_book_notifications.empty() && add_order_notifications.empty() && delete_order_notifications.empty() &&
               execute_order_notifications.empty() && update_order_notifications.empty();
    }

protected:
    void onOrderAdded(const AddedOrder &notification) override
    {
        add_order_notifications.push(notification);
    }
    void onOrderDeleted(const DeletedOrder &notification) override
    {
        delete_order_notifications.push(notification);
    }
    void onOrderUpdated(const UpdatedOrder &notification) override
    {
        update_order_notifications.push(notification);
    }
    void onOrderExecuted(const ExecutedOrder &notification) override
    {
        execute_order_notifications.push(notification);
    }
    void onSymbolAdded(const AddedSymbol &notification) override
    {
        add_symbol_notifications.push(notification);
    }
    void onSymbolDeleted(const DeletedSymbol &notification) override
    {
        delete_symbol_notifications.push(notification);
    }
    void onOrderBookAdded(const AddedOrderBook &notification) override
    {
        add_book_notifications.push(notification);
    }
    void onOrderBookDeleted(const DeletedOrderBook &notification) override
    {
        delete_book_notifications.push(notification);
    }
};
#endif // RAPID_TRADER_DEBUG_NOTIFICATION_PROCESSOR_H
