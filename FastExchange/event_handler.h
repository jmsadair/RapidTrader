#ifndef FAST_EXCHANGE_MAIN_EVENT_HANDLER_H
#define FAST_EXCHANGE_MAIN_EVENT_HANDLER_H
#include <iostream>
#include "event.h"
#include "receiver.h"
class EventHandler {
public:
    /**
     * Prepare the event handler for incoming messages.
     */
    void start() {
        try {
            while (true) {
                event_receiver.wait()
                    .handle<Message::Event::TradeEvent>([&](Message::Event::TradeEvent &msg)
                    {
                        //std::cout << msg << "\n" << std::endl;
                    })
                    .handle<Message::Event::OrderExecuted>([&](Message::Event::OrderExecuted &msg)
                    {
                        //std::cout << msg << "\n" << std::endl;
                    })
                    .handle<Message::Event::RejectionEvent>([&](Message::Event::RejectionEvent &msg)
                    {
                        //std::cout << msg << "\n" << std::endl;
                    });
            }
        } catch(const Messaging::CloseQueue&) {}
    }

    /**
     * @return a messenger that is capable of sending messages to the event handler.
     */
    inline Messaging::Sender getSender() {
        return static_cast<Messaging::Sender>(event_receiver);
    }

    /**
     * Shutdown the event handler.
     */
    void stop() {
        getSender().send(Messaging::CloseQueue());
    }
private:
    Messaging::Receiver event_receiver;
};
#endif //FAST_EXCHANGE_MAIN_EVENT_HANDLER_H
