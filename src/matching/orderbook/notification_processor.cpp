#include "notification_processor.h"

void NotificationProcessor::processNotifications()
{
    try
    {
        while (true)
        {
            notification_receiver.wait()
                .handle<AddedSymbol>([&](const AddedSymbol &notification) { onSymbolAdded(notification); })
                .handle<DeletedSymbol>([&](const DeletedSymbol &notification) { onSymbolDeleted(notification); })
                .handle<AddedOrderBook>([&](const AddedOrderBook &notification) { onOrderBookAdded(notification); })
                .handle<DeletedOrderBook>([&](const DeletedOrderBook &notification) { onOrderBookDeleted(notification); })
                .handle<AddedOrder>([&](const AddedOrder &notification) { onOrderAdded(notification); })
                .handle<DeletedOrder>([&](const DeletedOrder &notification) { onOrderDeleted(notification); })
                .handle<ExecutedOrder>([&](const ExecutedOrder &notification) { onOrderExecuted(notification); })
                .handle<UpdatedOrder>([&](const UpdatedOrder &notification) { onOrderUpdated(notification); });
        }
    }
    catch (const Messaging::CloseQueue &)
    {}
}