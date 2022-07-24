#ifndef RAPID_TRADER_RECEIVER_H
#define RAPID_TRADER_RECEIVER_H
#include "concurrent/messaging/dispatcher.h"
#include "sender.h"

namespace Concurrent::Messaging {
class Receiver
{
public:
    explicit operator Sender()
    {
        return Sender(&msg_queue);
    }

    Dispatcher wait()
    {
        return Dispatcher(&msg_queue);
    }

private:
    MessageQueue msg_queue;
};
} // namespace Concurrent::Messaging
#endif // RAPID_TRADER_RECEIVER_H
