#include "matching_engine.h"
#include "log.h"

using namespace Matching;

void MatchingEngine::start()
{
    try
    {
        while (true)
        {
            receiver.wait()
                .handle<Command::PlaceOrder>([&](Command::PlaceOrder &msg) { processCommand(msg); })
                .handle<Command::AddOrderBook>([&](Command::AddOrderBook &msg) { processCommand(msg); })
                .handle<Command::CancelOrder>([&](Command::CancelOrder &msg) { processCommand(msg); });
        }
    }
    catch (const Messaging::CloseQueue &)
    {}
}

void MatchingEngine::processCommand(const Command::PlaceOrder &command)
{
    if (command.order_symbol_id < symbol_to_book.size() && symbol_to_book[command.order_symbol_id].get())
    {
        Order order = command.makeOrder();
        symbol_to_book[command.order_symbol_id]->placeOrder(order);
    } else {
        LOG_WARN("Failed to place order - symbol does not exist.");
    }
}

void MatchingEngine::processCommand(const Command::CancelOrder &command)
{
    if (command.order_symbol_id < symbol_to_book.size() && symbol_to_book[command.order_symbol_id].get())
        symbol_to_book[command.order_symbol_id]->cancelOrder(command.order_id);
    else
        LOG_WARN("Failed to cancel order - symbol does not exist.");
}

void MatchingEngine::processCommand(const Command::ReduceOrder &command)
{
    if (command.order_symbol_id < symbol_to_book.size() && symbol_to_book[command.order_symbol_id].get())
        symbol_to_book[command.order_symbol_id]->reduceOrder(command.order_id, command.quantity_to_reduce_by);
    else
        LOG_WARN("Failed to reduce order - symbol does not exist.");
}

void MatchingEngine::processCommand(const Command::AddOrderBook &command)
{
    if (command.orderbook_symbol_id >= symbol_to_book.size() || !symbol_to_book[command.orderbook_symbol_id].get())
    {
        symbol_to_book.resize(command.orderbook_symbol_id + 1);
        symbol_to_book[command.orderbook_symbol_id] =
            std::make_unique<OrderBook::VectorOrderBook>(command.orderbook_symbol_id, orderbook_event_handler);
    }
    else
    {
        LOG_WARN("Failed to add orderbook - orderbook already exists.");
    }
}
