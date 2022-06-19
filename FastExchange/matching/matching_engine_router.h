#ifndef FAST_EXCHANGE_MAIN_MATCHING_ENGINE_ROUTER_H
#define FAST_EXCHANGE_MAIN_MATCHING_ENGINE_ROUTER_H
#include <robin_hood.h>
#include <shared_mutex>
#include <mutex>
#include <thread>
#include "matching_engine.h"

namespace Matching {
    class MatchingEngineRouter {
    public:
        /**
         * Submits a command to place an order to a matching engine.
         *
         * @param command a command to place an order, require that the symbol ID
         *                associated with the command exists.
         */
        inline void submitCommand(const Message::Command::PlaceOrder &command) {
            // Any number of threads can read the symbol_to_sender map as long
            // as another thread is not writing to the map.
            std::shared_lock<std::shared_mutex> lock(m);
            symbol_to_sender[command.order_symbol_id].send(command);
        }

        /**
         * Submits a command to cancel an order to a matching engine.
         *
         * @param command a command to cancel an order, requires that the symbol ID
         *                associated with the command exists.
         */
        inline void submitCommand(const Message::Command::CancelOrder &command) {
            // Any number of threads can read the symbol_to_sender map as long
            // as another thread is not writing to the map.
            std::shared_lock<std::shared_mutex> lock(m);
            symbol_to_sender[command.order_symbol_id].send(command);
        }

        /**
         * Submits a command to create a new order book to a matching engine.
         *
         * @param command a command to create a new order book.
         */
        inline void submitCommand(const Message::Command::AddOrderBook &command) {
            // Only one thread can write to symbol_to_sender map at a time, and no other threads can read the
            // map while this write is occurring.
            std::unique_lock<std::shared_mutex> lock(m);
            auto [it, success] = symbol_to_sender.insert(
                    {command.orderbook_symbol_id, senders[symbol_submission_index]});
            it->second.send(command);
            (++symbol_submission_index) %= static_cast<int>(senders.size());
        }

        /**
         * Adds a messenger that is capable of communicating with a matching engine.
         *
         * @param sender a messenger capable of communicating with a matching engine.
         */
        inline void addSender(Messaging::Sender sender) { senders.push_back(sender); }

    private:
        // Determines which matching engine a command needs to be submitted to.
        // Maps symbol IDs to senders.
        robin_hood::unordered_map<uint32_t, Messaging::Sender> symbol_to_sender;
        // All the senders that are communicating with the matching engines.
        std::vector<Messaging::Sender> senders;
        // Tracks the sender that is responsible for sending the command to add a new order book.
        // Each time a command to a new order book is submitted, this index is incremented so that
        // symbols are distributed to matching engines in a round-robin fashion.
        int symbol_submission_index = 0;
        std::shared_mutex m;
    };
}
#endif //FAST_EXCHANGE_MAIN_MATCHING_ENGINE_ROUTER_H
