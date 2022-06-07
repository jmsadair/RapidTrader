#ifndef FAST_EXCHANGE_EXCHANGE_API_H
#define FAST_EXCHANGE_EXCHANGE_API_H
#include "receiver.h"
#include "command.h"

class ExchangeApi {
public:
    /**
     * A constructor for the ExchangeApi ADT.
     *
     * @param orderbook_router_sender_ a messenger that is capable of sending
     *                                 messages to the event handler.
     */
    explicit ExchangeApi(Messaging::Sender orderbook_router_sender_) :
        orderbook_router_sender(orderbook_router_sender_)
    {}

    /**
     * Handles a user submitted command to an order book.
     *
     * @param command a command to an order book.
     */
    inline void submitCommand(Message::Command::AddOrderBook& command) {
        orderbook_router_sender.send(command);
    }

    /**
     * Handles a user submitted command to place an order.
     *
     * @param command a command to place an order.
     */
    inline void submitCommand(Message::Command::PlaceOrder& command) {
        orderbook_router_sender.send(command);
    }

    /**
     * Handles a user submitted command to cancel an order.
     *
     * @param command a command to cancel an order.
     */
    inline void submitCommand(Message::Command::CancelOrder& command) {
        orderbook_router_sender.send(command);
    }
private:
    Messaging::Sender orderbook_router_sender;
};
#endif //FAST_EXCHANGE_EXCHANGE_API_H
