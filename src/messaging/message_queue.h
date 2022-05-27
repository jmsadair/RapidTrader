#ifndef FAST_EXCHANGE_MESSAGE_QUEUE_H
#define FAST_EXCHANGE_MESSAGE_QUEUE_H
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

namespace messaging {
    /**
     * A message base class.
     */
    struct Message {
        virtual ~Message() = default;
    };

    /**
     * A wrapper class for the message base class that allows
     * for different types of messages.
     *
     * @tparam Msg the message type.
     */
    template<typename Msg>
    struct WrappedMessage: Message {
        Msg content;
        explicit WrappedMessage(const Msg& msg_content)
            : content(msg_content)
        {}
    };

    /**
     * A FIFO queue for messages.
     */
    class MessageQueue {
    public:
        /**
         * Pushes a message onto the queue.
         *
         * @tparam T the type of the message being pushed onto the queue.
         * @param msg the message to push onto the queue.
         */
        template <typename T>
        void push(const T& msg) {
            std::lock_guard<std::mutex> lk(m);
            message_queue.push(std::make_shared<WrappedMessage<T>>(msg));
            c.notify_all();
        }

        /**
         * Pops a message off the queue.
         *
         * @return the oldest message from the queue.
         */
        std::shared_ptr<Message> wait_and_pop() {
            std::unique_lock<std::mutex> lk(m);
            // Checks if queue is empty.
            const auto is_empty = [&]{ return !message_queue.empty(); };
            // Wait until queue is not empty.
            c.wait(lk, is_empty);
            auto msg = message_queue.front();
            message_queue.pop();
            return msg;
        }

    private:
        std::mutex m;
        std::condition_variable c;
        // Stores pointers to messages.
        std::queue<std::shared_ptr<Message>> message_queue;
    };
}
#endif //FAST_EXCHANGE_MESSAGE_QUEUE_H
