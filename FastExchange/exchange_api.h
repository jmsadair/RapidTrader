#ifndef FAST_EXCHANGE_EXCHANGE_API_H
#define FAST_EXCHANGE_EXCHANGE_API_H
#include <utility>
#include "matching_engine_router.h"
#include "command.h"

namespace FastExchange {
class ExchangeApi
{
public:
    /**
     * A constructor for the ExchangeApi ADT.
     *
     * @param orderbook_router_sender_ a messenger that is capable of sending
     *                                 messages to the event handler.
     */
    explicit ExchangeApi(std::shared_ptr<Matching::MatchingEngineRouter> engine_router_ptr_)
        : engine_router_ptr(std::move(engine_router_ptr_))
    {}

    /**
     * Handles a user submitted command to an order book.
     *
     * @param command a command to an order book.
     */
    inline void submitCommand(Message::Command::AddOrderBook &command)
    {
        engine_router_ptr->submitCommand(command);
    }

    /**
     * Handles a user submitted command to place an order.
     *
     * @param command a command to place an order.
     */
    inline void submitCommand(Message::Command::PlaceOrder &command)
    {
        engine_router_ptr->submitCommand(command);
    }

    /**
     * Handles a user submitted command to cancel an order.
     *
     * @param command a command to cancel an order.
     */
    inline void submitCommand(Message::Command::CancelOrder &command)
    {
        engine_router_ptr->submitCommand(command);
    }

private:
    std::shared_ptr<Matching::MatchingEngineRouter> engine_router_ptr;
};
} // namespace FastExchange
#endif // FAST_EXCHANGE_EXCHANGE_API_H
