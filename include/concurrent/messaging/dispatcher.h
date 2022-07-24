#ifndef RAPID_TRADER_DISPATCHER_H
#define RAPID_TRADER_DISPATCHER_H
#include "template_dispatcher.h"
#include <exception>

namespace Concurrent::Messaging {
// Command for closing the queue.
struct CloseQueue : std::exception
{};

class Dispatcher
{
public:
    Dispatcher(const Dispatcher &) = delete;
    Dispatcher &operator=(const Dispatcher &) = delete;

    Dispatcher(Dispatcher &&other) noexcept
        : msg_queue_ptr(other.msg_queue_ptr)
        , chained(other.chained)
    {
        other.chained = true;
    }

    explicit Dispatcher(MessageQueue *msg_queue_ptr_)
        : msg_queue_ptr(msg_queue_ptr_)
        , chained(false)
    {}

    template<typename Message, typename Func>
    TemplateDispatcher<Dispatcher, Message, Func> handle(Func &&func)
    {
        return TemplateDispatcher<Dispatcher, Message, Func>(msg_queue_ptr, this, std::forward<Func>(func));
    }

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

    [[noreturn]] void waitAndDispatch()
    {
        while (true)
        {
            // Gives up ownership of the message.
            dispatch(std::move(msg_queue_ptr->waitAndPop()));
        }
    }

    static bool dispatch(std::unique_ptr<BaseMessage> msg)
    {
        if (dynamic_cast<WrappedMessage<CloseQueue> *>(msg.get()))
        {
            throw CloseQueue();
        }
        return false;
    }
};
} // namespace Concurrent::Messaging
#endif // RAPID_TRADER_DISPATCHER_H
