#ifndef RAPID_TRADER_RECEIVER_H
#define RAPID_TRADER_RECEIVER_H
#include "concurrent/messaging/dispatcher.h"
#include "sender.h"

namespace Concurrent::Messaging {
/**
 * An ADT responsible for receiving message.
 */
class Receiver
{
public:
    /**
     * @return a sender that references the message queue.
     */
    inline explicit operator Sender()
    {
        return Sender(&msg_queue);
    }

    /**
     * @return a dispatcher that references the message queue.
     */
    inline Dispatcher wait()
    {
        return Dispatcher(&msg_queue);
    }

private:
    // The Receiver class owns the message queue.
    MessageQueue msg_queue;
};
} // namespace Concurrent::Messaging
#endif // RAPID_TRADER_RECEIVER_H