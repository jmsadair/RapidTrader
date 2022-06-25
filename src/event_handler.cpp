#include "event_handler.h"

void FastExchange::EventHandler::start()
{
    try
    {
        while (true)
        {
            event_receiver.wait()
                .handle<Message::Event::TradeEvent>([&](Message::Event::TradeEvent &msg) {
                    // std::cout << msg << "\n" << std::endl;
                })
                .handle<Message::Event::OrderExecuted>([&](Message::Event::OrderExecuted &msg) {
                    // std::cout << msg << "\n" << std::endl;
                })
                .handle<Message::Event::RejectionEvent>([&](Message::Event::RejectionEvent &msg) {
                    // std::cout << msg << "\n" << std::endl;
                });
        }
    }
    catch (const Messaging::CloseQueue &)
    {}
}
