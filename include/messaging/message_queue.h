#ifndef FAST_EXCHANGE_MESSAGE_QUEUE_H
#define FAST_EXCHANGE_MESSAGE_QUEUE_H
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

namespace Messaging {
/**
 * A message base class.
 */
struct BaseMessage
{
    virtual ~BaseMessage() = default;
};

/**
 * A wrapper class for the message base class that allows
 * for different types of message.
 *
 * @tparam Msg the message type.
 */
template<typename Msg>
struct WrappedMessage : BaseMessage
{
    Msg content;
    explicit WrappedMessage(const Msg &msg_content)
        : content(msg_content)
    {}
};

/**
 * A FIFO queue for message.
 */
class MessageQueue
{
public:
    /**
     * Pushes a message onto the queue.
     *
     * @tparam T the type of the message being pushed onto the queue.
     * @param msg the message to push onto the queue.
     */
    template<typename T>
    inline void push(const T &msg)
    {
        std::lock_guard<std::mutex> lk(m);
        message_queue.push(std::make_unique<WrappedMessage<T>>(msg));
        c.notify_all();
    }

    /**
     * Pops a message off the queue.
     *
     * @return the oldest message from the queue.
     */
    inline std::unique_ptr<BaseMessage> waitAndPop()
    {
        std::unique_lock<std::mutex> lk(m);
        // Wait until queue is not empty.
        c.wait(lk, [&]{ return !message_queue.empty(); });
        auto msg_ptr = std::move(message_queue.front());
        message_queue.pop();
        return msg_ptr;
    }

private:
    std::mutex m;
    std::condition_variable c;
    // Stores pointers to message.
    std::queue<std::unique_ptr<BaseMessage>> message_queue;
};
} // namespace Messaging
#endif // FAST_EXCHANGE_MESSAGE_QUEUE_H
