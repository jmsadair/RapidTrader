#ifndef FAST_EXCHANGE_SENDER_H
#define FAST_EXCHANGE_SENDER_H
#include "message_queue.h"

namespace Messaging {
/**
 * A wrapper for MessageQueue that only allows message
 * to be pushed onto the queue.
 */
class Sender
{
public:
    /**
     * A constructor for the Sender ADT.
     */
    Sender()
        : message_queue_ptr(nullptr)
    {}

    /**
     * A constructor for the sender ADT.
     *
     * @param queue_ptr a pointer to a message queue.
     */
    explicit Sender(MessageQueue *queue_ptr)
        : message_queue_ptr(queue_ptr)
    {}

    /**
     * Sends a message.
     *
     * @tparam Msg the type of the message that is to be sent.
     * @param msg the message that is to be sent.
     */
    template<typename Msg>
    void send(const Msg &msg)
    {
        if (message_queue_ptr)
            message_queue_ptr->push(msg);
    }

private:
    MessageQueue *message_queue_ptr;
};
} // namespace Messaging
#endif // FAST_EXCHANGE_SENDER_H
