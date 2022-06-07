#ifndef FAST_EXCHANGE_EXCHANGE_H
#define FAST_EXCHANGE_EXCHANGE_H
#include <thread>
#include "orderbook_router.h"
#include "exchange_api.h"
#include "event_handler.h"
class Exchange {
public:
    /**
     * A constructor for the Exchange ADT - spawns threads for the order matching engine and
     * the event handler.
     */
    Exchange() :
        orderbook_router(event_handler.getSender()), exchange_api(orderbook_router.getSender())
    {
        t1 = std::thread(&OrderBookRouter::start, &orderbook_router);
        t2 = std::thread(&EventHandler::start, &event_handler);
    }

    /**
     * A destructor for the Exchange ADT - kills the threads that order matching engine and event handler
     * were running on.
     */
    ~Exchange() {
        orderbook_router.stop();
        event_handler.stop();
        if (t1.joinable())
            t1.join();
        if (t2.joinable())
            t2.join();
    }

    /**
     * @return an API for the exchange.
     */
    inline ExchangeApi& getApi() {
        return exchange_api;
    }
private:
    EventHandler event_handler;
    OrderBookRouter orderbook_router;
    ExchangeApi exchange_api;
    std::thread t1;
    std::thread t2;
};
#endif //FAST_EXCHANGE_EXCHANGE_H
