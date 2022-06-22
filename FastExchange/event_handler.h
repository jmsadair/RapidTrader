#ifndef FAST_EXCHANGE_MAIN_EVENT_HANDLER_H
#define FAST_EXCHANGE_MAIN_EVENT_HANDLER_H
#include <iostream>
#include "event.h"
#include "receiver.h"
namespace FastExchange {
class EventHandler
{
public:
    /**
     * Prepare the event handler for incoming messages.
     */
    void start();

    /**
     * @return a messenger that is capable of sending messages to the event handler.
     */
    inline Messaging::Sender getSender()
    {
        return static_cast<Messaging::Sender>(event_receiver);
    }

    /**
     * Shutdown the event handler.
     */
    inline void stop()
    {
        getSender().send(Messaging::CloseQueue());
    }

private:
    Messaging::Receiver event_receiver;
};
} // namespace FastExchange
#endif // FAST_EXCHANGE_MAIN_EVENT_HANDLER_H
