#ifndef FAST_EXCHANGE_EXCHANGE_H
#define FAST_EXCHANGE_EXCHANGE_H
#include <thread>
#include "matching_engine.h"
#include "exchange_api.h"
#include "basic_event_handler.h"

namespace FastExchange {
class Exchange
{
public:
    /**
     * A constructor for the Exchange ADT - spawns the thread that the matching engine will run on.
     */
    Exchange();

    /**
     * A destructor for the Exchange ADT - kills the threads that the matching engine is running on.
     */
    ~Exchange();

    /**
     * @return an API for the exchange.
     */
    inline ExchangeApi &getApi()
    {
        return api;
    }

private:
    BasicEventHandler matching_engine_event_handler;
    std::shared_ptr<Matching::MatchingEngine> matching_engine;
    // Directs incoming commands to correct matching engine - thread safe.
    ExchangeApi api;
    std::thread matching_engine_thread;
};
} // namespace FastExchange
#endif // FAST_EXCHANGE_EXCHANGE_H
