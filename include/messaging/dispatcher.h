#ifndef FAST_EXCHANGE_DISPATCHER_H
#define FAST_EXCHANGE_DISPATCHER_H
#include "template_dispatcher.h"
#include <exception>

namespace Messaging {
// Message for closing the queue.
struct CloseQueue : std::exception
{};

/**
 * An ADT responsible for dispatching message.
 */
class Dispatcher
{
public:
    // Dispatcher ADT is move only.
    Dispatcher(const Dispatcher &) = delete;
    Dispatcher &operator=(const Dispatcher &) = delete;

    /**
     * A move constructor for the Dispatcher ADT.
     *
     * @param other another Dispatcher.
     */
    inline Dispatcher(Dispatcher &&other) noexcept
        : msg_queue_ptr(other.msg_queue_ptr)
        , chained(other.chained)
    {
        other.chained = true;
    }

    /**
     * A constructor for the Dispatcher ADT.
     *
     * @param msg_queue_ptr_ a pointer to a message queue.
     */
    inline explicit Dispatcher(MessageQueue *msg_queue_ptr_)
        : msg_queue_ptr(msg_queue_ptr_)
        , chained(false)
    {}

    /**
     * Handles a specific type of message.
     *
     * @tparam Message the type of the message to handle.
     * @tparam Func the type of the function to that will handle the message.
     * @param func the function that will handle the message.
     * @return a TemplateDispatcher capable of handling the message.
     */
    template<typename Message, typename Func>
    inline TemplateDispatcher<Dispatcher, Message, Func> handle(Func &&func)
    {
        return TemplateDispatcher<Dispatcher, Message, Func>(msg_queue_ptr, this, std::forward<Func>(func));
    }

    /**
     * A destructor for the Dispatcher ADT.
     */
    ~Dispatcher() noexcept(false)
    {
        if (!chained)
            waitAndDispatch();
    }

private:
    MessageQueue *msg_queue_ptr;
    bool chained;
    template<typename Dispatcher, typename Msg, typename Func>
    friend class TemplateDispatcher;

    /**
     * Waits for and dispatches message.
     */
    [[noreturn]] inline void waitAndDispatch()
    {
        while (true)
        {
            auto msg = msg_queue_ptr->waitAndPop();
            dispatch(msg);
        }
    }

    /**
     * Dispatches a message.
     *
     * @param msg the message to be dispatched.
     * @return false if the message was unhandled.
     * @throws Exception if a CloseQueue Message is received.
     */
    static inline bool dispatch(const std::shared_ptr<BaseMessage> &msg)
    {
        if (dynamic_cast<WrappedMessage<CloseQueue> *>(msg.get()))
        {
            throw CloseQueue();
        }
        return false;
    }
};
} // namespace Messaging
#endif // FAST_EXCHANGE_DISPATCHER_H
