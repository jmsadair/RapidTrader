#include "exchange.h"

void FastExchange::Exchange::start() {
    if (started) {
        return;
    }
    for (int i = 0; i < num_event_handlers; ++i) {
        event_handlers.push_back(std::make_shared<EventHandler>());
        event_handler_threads.emplace_back(&EventHandler::start, event_handlers.back());
    }
    for (int i = 0; i < num_engines; ++i)
    {
        engines.push_back(std::make_shared<Matching::MatchingEngine>(event_handlers[i % num_event_handlers]->getSender()));
        engine_threads.emplace_back(&Matching::MatchingEngine::start, engines.back());
        engine_router_ptr->addSender(engines.back()->getSender());
    }
    started = true;
}

FastExchange::Exchange::~Exchange()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // Close out matching engine message queues.
    for (auto &engine : engines)
    {
        engine->stop();
    }
    // Close out event handler message queues.
    for (auto &event_handler : event_handlers) {
        event_handler->stop();
    }
    // Join the threads the engines are running on.
    for (auto &engine_thread : engine_threads)
    {
        if (engine_thread.joinable())
            engine_thread.join();
    }
    // Join the threads the event handlers are running on.
    for (auto &event_handler_thread : event_handler_threads)
    {
        if (event_handler_thread.joinable())
            event_handler_thread.join();
    }
}