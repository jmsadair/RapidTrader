#ifndef RAPID_TRADER_EVENT_HANDLER_H
#define RAPID_TRADER_EVENT_HANDLER_H
#include <thread>
#include "concurrent/messaging/receiver.h"
#include "event_handler/event.h"

class EventHandler
{
public:
    EventHandler(const EventHandler &) = delete;
    EventHandler(EventHandler &&) = delete;
    EventHandler &operator=(const EventHandler &) = delete;
    EventHandler &operator=(EventHandler &&) = delete;

    /**
     * A constructor for the event handler. Spawns a thread to start
     * handling events.
     */
    EventHandler();

    /**
     * A destructor for the event handler. Joins the event handling
     * thread if it is joinable.
     */
    virtual ~EventHandler();

    /**
     * Spawns a thread to handle events, require that the event
     * handler is not already running.
     */
    void start();

    /**
     * Joins the thread handling events, require that there is
     * a thread handling events.
     */
    void stop();

    /**
     * @return a sender that can send messages to thread
     *         handling events.
     */
    Concurrent::Messaging::Sender getSender();

protected:
    // LCOV_EXCL_START
    /**
     * Handles a notification that an order was added.
     *
     * @param notification a notification that an order was added.
     */
    virtual void handleOrderAdded(const OrderAdded &notification) {}

    /**
     * Handles a notification that an order was deleted.
     *
     * @param notification a notification that an order was deleted.
     */
    virtual void handleOrderDeleted(const OrderDeleted &notification) {}

    /**
     * Handles a notification that an order was updated.
     *
     * @param notification a notification that an order was updated.
     */
    virtual void handleOrderUpdated(const OrderUpdated &notification) {}

    /**
     * Handles a notification that an order was executed.
     *
     * @param notification a notification that an order was executed.
     */
    virtual void handleOrderExecuted(const ExecutedOrder &notification) {}

    /**
     * Handles a notification that a symbol was added.
     *
     * @param notification a notification that a symbol was added.
     */
    virtual void handleSymbolAdded(const SymbolAdded &notification) {}

    /**
     * Handles a notification that a symbol was deleted.
     *
     * @param notification a notification that a symbol was deleted.
     */
    virtual void handleSymbolDeleted(const SymbolDeleted &notification) {}

    /**
     * Handles a notification that an orderbook was added.
     *
     * @param notification a notification that an orderbook was added.
     */
    virtual void handleOrderBookAdded(const OrderBookAdded &notification) {}

    /**
     * Handles a notification that an orderbook was deleted.
     *
     * @param notification a notification that an orderbook was deleted.
     */
    virtual void handleOrderBookDeleted(const OrderBookDeleted &notification) {}
    // LCOV_EXCL_STOP

private:
    /**
     * Processes incoming notifications.
     */
    void handleEvents();

    // Receives incoming messages.
    Concurrent::Messaging::Receiver message_receiver;
    // The thread that the notifications are processed on.
    std::thread handling_thread;
    // Indicates whether a thread is currently handling events.
    bool is_running;
};
#endif // RAPID_TRADER_EVENT_HANDLER_H
