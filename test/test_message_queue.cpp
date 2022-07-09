#include <gtest/gtest.h>
#include <future>
#include "concurrent/messaging/message_queue.h"

// A simple message for testing purposes.
struct TestMessage
{
    std::string text;
};

TEST(MessageQueueTest, QueueShouldHandlePushAndPop)
{
    Concurrent::Messaging::MessageQueue queue;
    TestMessage msg1{"test1"};
    // The queue is empty - no message can be popped until message are pushed onto the queue.
    std::future<std::unique_ptr<Concurrent::Messaging::BaseMessage>> msg1_future =
        std::async(std::launch::async, &Concurrent::Messaging::MessageQueue::waitAndPop, &queue);
    queue.push(msg1);
    // BaseMessage has been added to the queue - calling get() on the future should return a pointer to msg1.
    auto msg1_ptr = msg1_future.get();
    auto msg1_from_queue = dynamic_cast<Concurrent::Messaging::WrappedMessage<TestMessage> *>(msg1_ptr.get());
    // Casting should not return a null pointer.
    ASSERT_NE(msg1_from_queue, nullptr);
    ASSERT_EQ(msg1_from_queue->content.text, "test1");
}