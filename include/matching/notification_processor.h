#ifndef RAPID_TRADER_NOTIFICATION_PROCESSOR_H
#define RAPID_TRADER_NOTIFICATION_PROCESSOR_H
#include <thread>
#include "concurrent/messaging/receiver.h"
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

    /**
     * Spawns a thread that will start processing incoming notifications.
     */
    inline void run()
    {
        processing_thread = std::thread{&NotificationProcessor::processNotifications, this};
    }

    /**
     * Stops the notification processor.
     */
    inline void shutdown()
    {
        getSender().send(Concurrent::Messaging::CloseQueue());
        processing_thread.join();
    }

    /**
     * @return a message sender that can communicate with the notification
     *         processor.
     */
    inline Concurrent::Messaging::Sender getSender()
    {
        return static_cast<Concurrent::Messaging::Sender>(notification_receiver);
    }

protected:
    // LCOV_EXCL_START

    /**
     * Handles a notification that an order was added.
     *
     * @param notification a notification that an order was added.
     */
    virtual void onOrderAdded(const AddedOrder &notification){};

    /**
     * Handles a notification that an order was deleted.
     *
     * @param notification a notification that an order was deleted.
     */
    virtual void onOrderDeleted(const DeletedOrder &notification){};

    /**
     * Handles a notification that an order was updated.
     *
     * @param notification a notification that an order was updated.
     */
    virtual void onOrderUpdated(const UpdatedOrder &notification){};

    /**
     * Handles a notification that an order was executed.
     *
     * @param notification a notification that an order was executed.
     */
    virtual void onOrderExecuted(const ExecutedOrder &notification){};

    /**
     * Handles a notification that a symbol was added.
     *
     * @param notification a notification that a symbol was added.
     */
    virtual void onSymbolAdded(const AddedSymbol &notification){};

    /**
     * Handles a notification that a symbol was deleted.
     *
     * @param notification a notification that a symbol was deleted.
     */
    virtual void onSymbolDeleted(const DeletedSymbol &notification){};

    /**
     * Handles a notification that an orderbook was added.
     *
     * @param notification a notification that an orderbook was added.
     */
    virtual void onOrderBookAdded(const AddedOrderBook &notification){};

    /**
     * Handles a notification that an orderbook was deleted.
     *
     * @param notification a notification that an orderbook was deleted.
     */
    virtual void onOrderBookDeleted(const DeletedOrderBook &notification){};

    // LCOV_EXCL_STOP
private:
    /**
     * Processes incoming notifications.
     */
    void processNotifications();

    // The thread that the notifications are processed on.
    std::thread processing_thread;
    // Receives incoming notifications.
    Concurrent::Messaging::Receiver notification_receiver;
};
#endif // RAPID_TRADER_NOTIFICATION_PROCESSOR_H
