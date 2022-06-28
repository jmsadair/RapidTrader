#ifndef FAST_EXCHANGE_ORDERBOOK_ROUTER_H
#define FAST_EXCHANGE_ORDERBOOK_ROUTER_H
#include <iostream>
#include "receiver.h"
#include "sender.h"
#include "vector_orderbook.h"
#include "event_handler_interface.h"

namespace Matching {
class MatchingEngine
{
public:
    /**
     * A constructor for the MatchingEngine ADT.
     *
     * @param orderbook_event_handler_ the sender that each order book will
     *                           use to send message.
     */
    inline explicit MatchingEngine(EventHandler &orderbook_event_handler_)
        : orderbook_event_handler(orderbook_event_handler_)
    {}

    /**
     * @return a sender that can be used to send messages to the order book router.
     */
    inline Messaging::Sender getSender() noexcept
    {
        return static_cast<Messaging::Sender>(receiver);
    }

    /**
     * Prepares the router to start receiving commands.
     */
    void start();

    /**
     * Shutdown the order book router.
     */
    inline void stop()
    {
        getSender().send(Messaging::CloseQueue());
    }

private:
    /**
     * Processes a command for placing an order.
     *
     * @param command a command to place an order.
     */
    void processCommand(const Command::PlaceOrder &command);

    /**
     * Processes a command for cancelling an order.
     *
     * @param command a command to cancel an order.
     */
    void processCommand(const Command::CancelOrder &command);

    /**
     * Processes a command for reducing an order.
     *
     * @param command a command to reduce an order.
     */
    void processCommand(const Command::ReduceOrder &command);

    /**
     * Processes a command for adding creating a new order book.
     *
     * @param command a command to create a new order book.
     */
    void processCommand(const Command::AddOrderBook &command);

    // Maps a symbol to its corresponding order book.
    std::vector<std::unique_ptr<OrderBook::VectorOrderBook>> symbol_to_book;
    // Receives incoming command messages.
    Messaging::Receiver receiver;
    // Sends messages from each order book to the event handler.
    EventHandler &orderbook_event_handler;
};
} // namespace Matching
#endif // FAST_EXCHANGE_ORDERBOOK_ROUTER_H
