#ifndef FAST_EXCHANGE_ORDERBOOK_ROUTER_H
#define FAST_EXCHANGE_ORDERBOOK_ROUTER_H
#include <iostream>
#include "orderbook/vector_orderbook.h"

namespace Matching {
    class MatchingEngine {
    public:
        /**
         * A constructor for the MatchingEngine ADT.
         *
         * @param orderbook_sender_ the sender that each order book will
         *                           use to send message.
         */
        inline explicit MatchingEngine(Messaging::Sender orderbook_sender_) : orderbook_sender(orderbook_sender_) {}

        /**
         * @return a sender that can be used to send messages to the order book router.
         */
        inline Messaging::Sender getSender() noexcept { return static_cast<Messaging::Sender>(receiver); }

        /**
         * Prepares the router to start receiving commands.
         */
        void start();

        /**
         * Shutdown the order book router.
         */
        inline void stop() { getSender().send(Messaging::CloseQueue()); }

    private:
        /**
         * Processes a command for placing an order.
         *
         * @param command a command to place an order.
         */
        void processCommand(const Message::Command::PlaceOrder &command);

        /**
         * Processes a command for cancelling an order.
         *
         * @param command a command to cancel an order.
         */
        void processCommand(const Message::Command::CancelOrder &command);

        /**
         * Processes a command for adding creating a new order book.
         *
         * @param command a command to create a new order book.
         */
        void processCommand(const Message::Command::AddOrderBook &command);

        // Maps a symbol to its corresponding order book.
        std::unordered_map<uint32_t, OrderBook::VectorOrderBook> symbol_to_book;
        // Receives incoming command messages.
        Messaging::Receiver receiver;
        // Sends messages from each order book to the event handler.
        Messaging::Sender orderbook_sender;
    };
}
#endif //FAST_EXCHANGE_ORDERBOOK_ROUTER_H
