#ifndef FAST_EXCHANGE_EXCHANGE_H
#define FAST_EXCHANGE_EXCHANGE_H
#include <thread>
#include "matching_engine.h"
#include "exchange_api.h"
#include "event_handler.h"
#include "log.h"

namespace FastExchange {
class Exchange
{
public:
    Exchange()
        : event_handler(std::make_shared<FastExchange::EventHandler>())
        , matching_engine(std::make_shared<Matching::MatchingEngine>(event_handler->getSender()))
        , api(matching_engine->getSender())
    {
        //Log::init();
        matching_engine_thread = std::thread(&Matching::MatchingEngine::start, matching_engine);
        event_handler_thread = std::thread(&FastExchange::EventHandler::start, event_handler);
        LOG_INFO("Exchange started...");
    }

    /**
     * A destructor for the Exchange ADT - kills the threads that order matching engine and event handler
     * were running on.
     */
    ~Exchange()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        matching_engine->stop();
        event_handler->stop();
        matching_engine_thread.join();
        event_handler_thread.join();
        LOG_INFO("Exchange stopped...");
    }

    /**
     * @return an API for the exchange.
     */
    inline ExchangeApi &getApi()
    {
        return api;
    }

private:
    std::shared_ptr<FastExchange::EventHandler> event_handler;
    std::shared_ptr<Matching::MatchingEngine> matching_engine;
    // Directs incoming commands to correct matching engine.
    ExchangeApi api;
    // True if engine has been started and false otherwise.
    std::thread matching_engine_thread;
    std::thread event_handler_thread;
};
} // namespace FastExchange
#endif // FAST_EXCHANGE_EXCHANGE_H
