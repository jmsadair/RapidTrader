#ifndef RAPID_TRADER_MESSAGE_QUEUE_H
#define RAPID_TRADER_MESSAGE_QUEUE_H
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

namespace Concurrent::Messaging {
struct BaseMessage
{
    virtual ~BaseMessage() = default;
};

template<typename Msg>
struct WrappedMessage : BaseMessage
{
    Msg content;
    explicit WrappedMessage(const Msg &msg_content)
        : content(msg_content)
    {}
};

/**
 * A thread-safe message queue.
 */
class MessageQueue
{
public:
    template<typename T>
    void push(const T &msg)
    {
        std::lock_guard<std::mutex> lk(m);
        message_queue.push(std::make_unique<WrappedMessage<T>>(msg));
        c.notify_all();
    }

    std::unique_ptr<BaseMessage> waitAndPop()
    {
        std::unique_lock<std::mutex> lk(m);
        // Wait until queue is not empty.
        c.wait(lk, [&] { return !message_queue.empty(); });
        auto msg_ptr = std::move(message_queue.front());
        message_queue.pop();
        return msg_ptr;
    }

private:
    std::mutex m;
    std::condition_variable c;
    std::queue<std::unique_ptr<BaseMessage>> message_queue;
};
} // namespace Concurrent::Messaging
#endif // RAPID_TRADER_MESSAGE_QUEUE_H
