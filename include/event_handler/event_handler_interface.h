#ifndef FAST_EXCHANGE_EVENT_HANDLER_INTERFACE_H
#define FAST_EXCHANGE_EVENT_HANDLER_INTERFACE_H
#include "event.h"

/**
 * Handles events that occur inside an order book - an order being traded, an order being
 * rejected, or an an order being executed in full.
 */
class EventHandler
{
public:
    /**
     * Handles a trade event.
     *
     * @param trade_event a trade event.
     */
    virtual void handleTradeEvent(const Event::TradeEvent &trade_event) = 0;

    /**
     * Handles a rejection event.
     *
     * @param reject_event a rejection event.
     */
    virtual void handleRejectEvent(const Event::RejectionEvent &reject_event) = 0;

    /**
     * Handles an execution event.
     *
     * @param execution_event an execution event.
     */
    virtual void handleExecutionEvent(const Event::OrderExecuted &execution_event) = 0;

    virtual ~EventHandler() = default;
};
#endif // FAST_EXCHANGE_EVENT_HANDLER_INTERFACE_H
