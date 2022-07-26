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
     * Joins the thread handling events, require that the event handler
     * is already running.
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
     * Handles a event that an order was added.
     *
     * @param event a event that an order was added.
     */
    virtual void handleOrderAdded(const OrderAdded &event) {}

    /**
     * Handles a event that an order was deleted.
     *
     * @param event a event that an order was deleted.
     */
    virtual void handleOrderDeleted(const OrderDeleted &event) {}

    /**
     * Handles a event that an order was updated.
     *
     * @param event a event that an order was updated.
     */
    virtual void handleOrderUpdated(const OrderUpdated &event) {}

    /**
     * Handles a event that an order was executed.
     *
     * @param event a event that an order was executed.
     */
    virtual void handleOrderExecuted(const ExecutedOrder &event) {}

    /**
     * Handles a event that a symbol was added.
     *
     * @param event a event that a symbol was added.
     */
    virtual void handleSymbolAdded(const SymbolAdded &event) {}

    /**
     * Handles a event that a symbol was deleted.
     *
     * @param event a event that a symbol was deleted.
     */
    virtual void handleSymbolDeleted(const SymbolDeleted &event) {}

    /**
     * Handles a event that an orderbook was added.
     *
     * @param event a event that an orderbook was added.
     */
    virtual void handleOrderBookAdded(const OrderBookAdded &event) {}

    /**
     * Handles a event that an orderbook was deleted.
     *
     * @param event a event that an orderbook was deleted.
     */
    virtual void handleOrderBookDeleted(const OrderBookDeleted &event) {}
    // LCOV_EXCL_STOP

private:
    /**
     * Processes incoming events.
     */
    void handleEvents();

    // Receives incoming messages.
    Concurrent::Messaging::Receiver message_receiver;
    // The thread that the events are processed on.
    std::thread handling_thread;
    // Indicates whether a thread is currently handling events.
    bool is_running;
};
#endif // RAPID_TRADER_EVENT_HANDLER_H
