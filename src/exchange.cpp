#include "exchange.h"
#include "log.h"

FastExchange::Exchange::Exchange()
    : matching_engine(std::make_shared<Matching::MatchingEngine>(matching_engine_event_handler))
    , api(matching_engine->getSender())
{
    //Log::init();
    matching_engine_thread = std::thread(&Matching::MatchingEngine::start, matching_engine);
    LOG_INFO("Exchange started...");
}

FastExchange::Exchange::~Exchange()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    matching_engine->stop();
    matching_engine_thread.join();
    LOG_INFO("Exchange stopped...");
}
