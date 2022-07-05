#ifndef RAPID_TRADER_NOTIFICATION_PROCESSOR_H
#define RAPID_TRADER_NOTIFICATION_PROCESSOR_H
#include <thread>
#include "receiver.h"
#include "notification.h"

class NotificationProcessor
{
public:
    NotificationProcessor() = default;
    virtual ~NotificationProcessor() = default;
    NotificationProcessor(const NotificationProcessor &) = delete;
    NotificationProcessor(NotificationProcessor &&) = delete;
    NotificationProcessor &operator=(const NotificationProcessor &) = delete;
    NotificationProcessor &operator=(NotificationProcessor &&) = delete;

    inline void run()
    {
        processing_thread = std::thread{&NotificationProcessor::processNotifications, this};
    }

    inline void shutdown()
    {
        getSender().send(Messaging::CloseQueue());
        processing_thread.join();
    }

    inline Messaging::Sender getSender()
    {
        return static_cast<Messaging::Sender>(notification_receiver);
    }

protected:
    virtual void onOrderAdded(const AddedOrder &notification){};
    virtual void onOrderDeleted(const DeletedOrder &notification){};
    virtual void onOrderUpdated(const UpdatedOrder &notification){};
    virtual void onOrderExecuted(const ExecutedOrder &notification){};
    virtual void onSymbolAdded(const AddedSymbol &notification){};
    virtual void onSymbolDeleted(const DeletedSymbol &notification){};
    virtual void onOrderBookAdded(const AddedOrderBook &notification){};
    virtual void onOrderBookDeleted(const DeletedOrderBook &notification){};

private:
    void processNotifications();
    std::thread processing_thread;
    Messaging::Receiver notification_receiver;
};
#endif // RAPID_TRADER_NOTIFICATION_PROCESSOR_H
