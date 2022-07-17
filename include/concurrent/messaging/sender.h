#ifndef RAPID_TRADER_SENDER_H
#define RAPID_TRADER_SENDER_H
#include "concurrent/messaging/message_queue.h"

namespace Concurrent::Messaging {
class Sender
{
public:
    inline Sender()
        : message_queue_ptr(nullptr)
    {}

    inline explicit Sender(MessageQueue *queue_ptr)
        : message_queue_ptr(queue_ptr)
    {}

    template<typename Msg>
    inline void send(const Msg &msg)
    {
        if (message_queue_ptr)
            message_queue_ptr->push(msg);
    }

private:
    MessageQueue *message_queue_ptr;
};
} // namespace Concurrent::Messaging
#endif // RAPID_TRADER_SENDER_H
