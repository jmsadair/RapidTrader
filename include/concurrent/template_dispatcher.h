#ifndef RAPID_TRADER_TEMPLATE_DISPATCHER_H
#define RAPID_TRADER_TEMPLATE_DISPATCHER_H
#include "message_queue.h"

namespace Messaging {
/**
 * An ADT that is capable of handling message of the specified type.
 *
 * @tparam PreviousDispatcher the type of the dispatcher that this dispatcher
 *                            is chained to.
 * @tparam Msg the type of the message that this dispatcher can handle.
 * @tparam Func the function that will handle the provided message.
 */
template<typename PreviousDispatcher, typename Msg, typename Func>
class TemplateDispatcher
{
public:
    // TemplateDispatcher ADT is move only.
    TemplateDispatcher(TemplateDispatcher const &) = delete;
    TemplateDispatcher &operator=(TemplateDispatcher const &) = delete;

    /**
     * A move constructor for the TemplateDispatcher ADT.
     *
     * @param other another TemplateDispatcher.
     */
    inline TemplateDispatcher(TemplateDispatcher &&other) noexcept
        : msg_queue_ptr(other.msg_queue_ptr)
        , prev_dispatcher_ptr(other.prev_dispatcher_ptr)
        , func(std::move(other.func))
        , chained(other.chained)
    {
        other.chained = true;
    }

    /**
     * A constructor for the TemplateDispatcher ADT.
     *
     * @param msg_queue_ptr_ a pointer to a message queue.
     * @param prev_dispatcher_ptr_ a pointer to another dispatcher.
     * @param func_ the function that will handle the message.
     */
    inline TemplateDispatcher(MessageQueue *msg_queue_ptr_, PreviousDispatcher *prev_dispatcher_ptr_, Func &&func_)
        : msg_queue_ptr(msg_queue_ptr_)
        , prev_dispatcher_ptr(prev_dispatcher_ptr_)
        , func(std::forward<Func>(func_))
        , chained(false)
    {
        prev_dispatcher_ptr->chained = true;
    }

    /**
     * Chains dispatchers to allow for multiple types of message to be handled.
     *
     * @tparam OtherMsg the type of the message.
     * @tparam OtherFunc the type of the function that will handle the message.
     * @param other_func the function that will handle the message.
     * @return a new TemplateDispatcher.
     */
    template<typename OtherMsg, typename OtherFunc>
    inline TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc> handle(OtherFunc &&other_func)
    {
        return TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc>(msg_queue_ptr, this, std::forward<OtherFunc>(other_func));
    }

    /**
     * A destructor for the TemplateDispatcher ADT.
     */
    ~TemplateDispatcher() noexcept(false)
    {
        // Note that this function is marked noexcept(false) since
        // any of the handlers might throw an exception.
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

    /**
     * Waits for a message and attempts to handle it.
     */
    inline void waitAndDispatch()
    {
        while (true)
        {
            // Break if the message was successfully handled.
            if (dispatch(std::move(msg_queue_ptr->waitAndPop())))
                break;
        }
    }

    /**
     * Calls the supplied function if the provided message is a match; otherwise,
     * if the message is not a match, the previous dispatcher is chained to.
     *
     * @param msg the message that the supplied function will be called on.
     * @return true if the if the message is a match.
     */
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
} // namespace Messaging
#endif // RAPID_TRADER_TEMPLATE_DISPATCHER_H
