#ifndef FAST_EXCHANGE_BASIC_EVENT_HANDLER_H
#define FAST_EXCHANGE_BASIC_EVENT_HANDLER_H
#include "event_handler_interface.h"
#include "log.h"
/**
 * A simple event handler used to track how many orders were executed, traded, and rejected.
 */
class BasicEventHandler : public EventHandler
{
public:
    /**
     * Increments the number of trade events.
     *
     * @param trade_event a trade event.
     */
    inline void handleTradeEvent(const Event::TradeEvent &trade_event) override
    {
        LOG_DEBUG("Trade event occurred.");
        ++num_trade_events;
    };

    /**
     * Increments the number of rejection events.
     *
     * @param reject_event a rejection event.
     */
    inline void handleRejectEvent(const Event::RejectionEvent &reject_event) override
    {
        LOG_DEBUG("Rejection event occurred.");
        ++num_rejection_events;
    };

    /**
     * Increments the number of execution events.
     *
     * @param execution_event an execution event.
     */
    inline void handleExecutionEvent(const Event::OrderExecuted &execution_event) override
    {
        LOG_DEBUG("Execution event occurred.");
        ++num_execution_events;
    };
private:
    uint64_t num_execution_events = 0;
    uint64_t num_trade_events = 0;
    uint64_t num_rejection_events = 0;
};
#endif // FAST_EXCHANGE_BASIC_EVENT_HANDLER_H
