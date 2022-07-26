#include "event_handler/event_handler.h"

EventHandler::EventHandler()
    : handling_thread(&EventHandler::handleEvents, this)
    , is_running(true)
{}

EventHandler::~EventHandler()
{
    if (handling_thread.joinable())
    {
        Concurrent::Messaging::Sender(message_receiver).send(Concurrent::Messaging::CloseQueue());
        handling_thread.join();
        is_running = false;
    }
}

void EventHandler::start()
{
    assert(!is_running && "Event handler is already running!");
    try
    {
        handling_thread = std::thread(&EventHandler::handleEvents, this);
        is_running = true;
    }
    //LCOV_EXCL_START
    catch (...)
    {
        is_running = false;
        throw;
    }
    //LCOV_EXCL_END
}

void EventHandler::stop()
{
    assert(is_running && "Event handler is not running!");
    if (handling_thread.joinable())
    {
        is_running = false;
        Concurrent::Messaging::Sender(message_receiver).send(Concurrent::Messaging::CloseQueue());
        handling_thread.join();
    }
}

Concurrent::Messaging::Sender EventHandler::getSender()
{
    return Concurrent::Messaging::Sender(message_receiver);
}
void EventHandler::handleEvents()
{
    try
    {
        while (true)
        {
            message_receiver.wait()
                .handle<SymbolAdded>([&](const SymbolAdded &notification) { handleSymbolAdded(notification); })
                .handle<SymbolDeleted>([&](const SymbolDeleted &notification) { handleSymbolDeleted(notification); })
                .handle<OrderBookAdded>([&](const OrderBookAdded &notification) { handleOrderBookAdded(notification); })
                .handle<OrderBookDeleted>([&](const OrderBookDeleted &notification) { handleOrderBookDeleted(notification); })
                .handle<OrderAdded>([&](const OrderAdded &notification) { handleOrderAdded(notification); })
                .handle<OrderDeleted>([&](const OrderDeleted &notification) { handleOrderDeleted(notification); })
                .handle<ExecutedOrder>([&](const ExecutedOrder &notification) { handleOrderExecuted(notification); })
                .handle<OrderUpdated>([&](const OrderUpdated &notification) { handleOrderUpdated(notification); });
        }
    }
    catch (const Concurrent::Messaging::CloseQueue &)
    {}
}