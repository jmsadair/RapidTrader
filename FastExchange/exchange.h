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
    /**
     * A constructor for the Exchange ADT - spawns threads for the order matching engine and
     * the event handler.
     *
     * @param num_engines_ the number of matching engines that the exchange will spawn, require that
     *                     num_engines_ is positive.
     * @param num_event_handlers_ the number of event handlers that the exchange will spawn, require that
     *                            num_event_handlers_ is positive.
     */
    explicit Exchange(uint8_t num_engines_ = 1, uint8_t num_event_handlers_ = 1)
        : engine_router_ptr(std::make_shared<Matching::MatchingEngineRouter>()), api(engine_router_ptr), num_engines(num_engines_),
            num_event_handlers(num_event_handlers_), started(false)
    {}

    /**
     * A destructor for the Exchange ADT - kills the threads that order matching engine and event handler
     * were running on.
     */
    ~Exchange();

    /**
     * Prepares the exchange to receive incoming commands, require that the exchange has
     * not already been started.
     */
    void start();

    /**
     * Indicates whether the exchange has been started.
     *
     * @return true if exchange has been started and false otherwise.
     */
    [[nodiscard]] inline bool isStarted() const {
        return started;
    }

    /**
     * @return an API for the exchange.
     */
    inline ExchangeApi &getApi()
    {
        return api;
    }

private:
    // Directs incoming commands to correct matching engine.
    std::shared_ptr<Matching::MatchingEngineRouter> engine_router_ptr;
    ExchangeApi api;
    // Number of matching engines that will be running.
    const uint8_t num_engines;
    // Number of event handlers that will be running.
    const uint8_t num_event_handlers;
    // True if engine has been started and false otherwise.
    bool started;
    std::vector<std::shared_ptr<Matching::MatchingEngine>> engines;
    std::vector<std::shared_ptr<EventHandler>> event_handlers;
    std::vector<std::thread> engine_threads;
    std::vector<std::thread> event_handler_threads;
};
} // namespace FastExchange
#endif // FAST_EXCHANGE_EXCHANGE_H
