#ifndef RAPID_TRADER_TEMPLATE_DISPATCHER_H
#define RAPID_TRADER_TEMPLATE_DISPATCHER_H
#include "concurrent/messaging/message_queue.h"

namespace Concurrent::Messaging {
template<typename PreviousDispatcher, typename Msg, typename Func>
class TemplateDispatcher
{
public:
    TemplateDispatcher(TemplateDispatcher const &) = delete;
    TemplateDispatcher &operator=(TemplateDispatcher const &) = delete;

    inline TemplateDispatcher(TemplateDispatcher &&other) noexcept
        : msg_queue_ptr(other.msg_queue_ptr)
        , prev_dispatcher_ptr(other.prev_dispatcher_ptr)
        , func(std::move(other.func))
        , chained(other.chained)
    {
        other.chained = true;
    }

    inline TemplateDispatcher(MessageQueue *msg_queue_ptr_, PreviousDispatcher *prev_dispatcher_ptr_, Func &&func_)
        : msg_queue_ptr(msg_queue_ptr_)
        , prev_dispatcher_ptr(prev_dispatcher_ptr_)
        , func(std::forward<Func>(func_))
        , chained(false)
    {
        prev_dispatcher_ptr->chained = true;
    }

    template<typename OtherMsg, typename OtherFunc>
    inline TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc> handle(OtherFunc &&other_func)
    {
        return TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc>(msg_queue_ptr, this, std::forward<OtherFunc>(other_func));
    }

    ~TemplateDispatcher() noexcept(false)
    {
        if (!chained)
            waitAndDispatch();
    }

private:
    MessageQueue *msg_queue_ptr;
    PreviousDispatcher *prev_dispatcher_ptr;
    Func func;
    bool chained;
    template<typename Dispatcher, typename OtherMsg, typename OtherFunc>
    friend class TemplateDispatcher;

    inline void waitAndDispatch()
    {
        while (true)
        {
            // Break if the message was successfully handled.
            if (dispatch(std::move(msg_queue_ptr->waitAndPop())))
                break;
        }
    }

    inline bool dispatch(std::unique_ptr<BaseMessage> msg)
    {
        auto *wrapped = dynamic_cast<WrappedMessage<Msg> *>(msg.get());
        if (wrapped)
        {
            func(wrapped->content);
            return true;
        }
        else
        {
            return prev_dispatcher_ptr->dispatch(std::move(msg));
        }
    }
};
} // namespace Concurrent::Messaging
#endif // RAPID_TRADER_TEMPLATE_DISPATCHER_H
