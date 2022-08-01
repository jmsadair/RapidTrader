#ifndef RAPID_TRADER_EVENT_HANDLER_H
#define RAPID_TRADER_EVENT_HANDLER_H
#include "market/market.h"
#include "market/concurrent_market.h"
#include "event_handler/event.h"

class EventHandler
{
public:
    EventHandler(const EventHandler &) = delete;
    EventHandler(EventHandler &&) = delete;
    EventHandler &operator=(const EventHandler &) = delete;
    EventHandler &operator=(EventHandler &&) = delete;
    EventHandler() = default;
    virtual ~EventHandler() = default;

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
    // LCOV_EXCL_STOP
};
#endif // RAPID_TRADER_EVENT_HANDLER_H
