#include "matching_engine.h"

void MatchingEngine::start() {
    try {
        while (true) {
            receiver.wait()
                .handle<Message::Command::PlaceOrder>([&](Message::Command::PlaceOrder &msg)
                {
                    processCommand(msg);
                })
                .handle<Message::Command::AddOrderBook>([&](Message::Command::AddOrderBook &msg)
                {
                    processCommand(msg);
                })
                .handle<Message::Command::CancelOrder>([&](Message::Command::CancelOrder &msg)
                {
                    processCommand(msg);
                });
        }
    } catch(const Messaging::CloseQueue&) {}
}

void MatchingEngine::processCommand(const Message::Command::PlaceOrder &command) {
    auto it = symbol_to_book.find(command.order_symbol_id);
    if (it != symbol_to_book.end()) {
        it->second.placeOrder(command.makeOrder());
    }
}

void MatchingEngine::processCommand(const Message::Command::CancelOrder &command) {
    auto it = symbol_to_book.find(command.order_symbol_id);
    if (it != symbol_to_book.end())
        it->second.cancelOrder(command.order_id);
}

void MatchingEngine::processCommand(const Message::Command::AddOrderBook &command) {
    auto it = symbol_to_book.find(command.orderbook_symbol_id);
    if (it == symbol_to_book.end())
        symbol_to_book.emplace(std::piecewise_construct, std::forward_as_tuple(command.orderbook_symbol_id),
                               std::forward_as_tuple(command.orderbook_symbol_id, orderbook_sender));
}
