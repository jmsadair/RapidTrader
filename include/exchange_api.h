#ifndef FAST_EXCHANGE_EXCHANGE_API_H
#define FAST_EXCHANGE_EXCHANGE_API_H
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
    explicit ExchangeApi(Messaging::Sender matching_engine_sender_)
        : matching_engine_sender(matching_engine_sender_)
    {}

    /**
     * Handles a user submitted command to an order book.
     *
     * @param command a command to an order book.
     */
    inline void submitCommand(Message::Command::AddOrderBook &command)
    {
        matching_engine_sender.send(command);
    }

    /**
     * Handles a user submitted command to place an order.
     *
     * @param command a command to place an order.
     */
    inline void submitCommand(Message::Command::PlaceOrder &command)
    {
        matching_engine_sender.send(command);
    }

    /**
     * Handles a user submitted command to reduce an order.
     *
     * @param command a command to reduce an order.
     */
    inline void submitCommand(Message::Command::ReduceOrder &command)
    {
        matching_engine_sender.send(command);
    }

    /**
     * Handles a user submitted command to cancel an order.
     *
     * @param command a command to cancel an order.
     */
    inline void submitCommand(Message::Command::CancelOrder &command)
    {
        matching_engine_sender.send(command);
    }

private:
    Messaging::Sender matching_engine_sender;
};
} // namespace FastExchange
#endif // FAST_EXCHANGE_EXCHANGE_API_H
