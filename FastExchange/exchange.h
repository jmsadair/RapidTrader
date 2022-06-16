#ifndef FAST_EXCHANGE_EXCHANGE_H
#define FAST_EXCHANGE_EXCHANGE_H
#include <thread>
#include "matching_engine.h"
#include "exchange_api.h"
#include "event_handler.h"
class Exchange {
public:
    /**
     * A constructor for the Exchange ADT - spawns threads for the order matching engine and
     * the event handler.
     */
    explicit Exchange(int num_engines) :
        api(engine_router_ptr)
    {
        engines.reserve(num_engines);
        threads.reserve(num_engines + 1);
        for (int i = 0; i < num_engines; ++i) {
            engines.push_back(std::make_shared<MatchingEngine>(event_handler.getSender()));
            threads.emplace_back(&MatchingEngine::start, engines.back());
            engine_router_ptr->addSender(engines.back()->getSender());
        }
        threads.emplace_back(&EventHandler::start, &event_handler);
    }

    /**
     * A destructor for the Exchange ADT - kills the threads that order matching engine and event handler
     * were running on.
     */
    ~Exchange() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        for (auto& engine : engines) {
            engine->stop();
        }
        event_handler.stop();
        for (auto& thread : threads) {
            if (thread.joinable())
                thread.join();
        }
    }

    /**
     * @return an API for the exchange.
     */
    inline ExchangeApi& getApi() { return api; }

private:
    EventHandler event_handler;
    std::shared_ptr<MatchingEngineRouter> engine_router_ptr = std::make_shared<MatchingEngineRouter>();
    ExchangeApi api;
    std::vector<std::shared_ptr<MatchingEngine>> engines;
    std::vector<std::thread> threads;
    std::vector<Messaging::Sender> senders;
};
#endif //FAST_EXCHANGE_EXCHANGE_H
