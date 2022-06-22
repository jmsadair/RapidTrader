#include "matching_engine.h"
#include <iostream>

using namespace Matching;

void MatchingEngine::start()
{
    try
    {
        while (true)
        {
            receiver.wait()
                .handle<Message::Command::PlaceOrder>([&](Message::Command::PlaceOrder &msg) { processCommand(msg); })
                .handle<Message::Command::AddOrderBook>([&](Message::Command::AddOrderBook &msg) { processCommand(msg); })
                .handle<Message::Command::CancelOrder>([&](Message::Command::CancelOrder &msg) { processCommand(msg); });
        }
    }
    catch (const Messaging::CloseQueue &)
    {}
}

void MatchingEngine::processCommand(const Message::Command::PlaceOrder &command)
{
    if (command.order_symbol_id < symbol_to_book.size())
    {
        Order order = command.makeOrder();
        symbol_to_book[command.order_symbol_id]->placeOrder(order);
    }
}

void MatchingEngine::processCommand(const Message::Command::CancelOrder &command)
{
    if (command.order_symbol_id < symbol_to_book.size())
        symbol_to_book[command.order_symbol_id]->cancelOrder(command.order_id);
}

void MatchingEngine::processCommand(const Message::Command::AddOrderBook &command)
{
    if (command.orderbook_symbol_id >= symbol_to_book.size())
        symbol_to_book.push_back(std::make_unique<OrderBook::VectorOrderBook>(command.orderbook_symbol_id, orderbook_sender));
}
