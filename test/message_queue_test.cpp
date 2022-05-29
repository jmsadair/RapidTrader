#include <gtest/gtest.h>
#include <future>
#include "message_queue.h"

// A simple message ADT for testing purposes.
struct TestMessage {
    std::string text;
};

TEST(MessageQueueTest, QueueShouldHandlePushAndPop) {
    Messaging::MessageQueue queue;
    TestMessage msg1 {"test1"};
    // The queue is empty - no messages can be popped until messages are pushed onto the queue.
    std::future<std::shared_ptr<Messaging::BaseMessage>> msg1_future =
            std::async(std::launch::async, &Messaging::MessageQueue::waitAndPop, &queue);
    queue.push(msg1);
    // BaseMessage has been added to the queue - calling get() on the future should return a pointer to msg1.
    auto msg1_ptr = msg1_future.get();
    auto msg1_from_queue = dynamic_cast<Messaging::WrappedMessage<TestMessage>*>(msg1_ptr.get());
    // Casting should not return a null pointer.
    ASSERT_NE(msg1_from_queue, nullptr);
    ASSERT_EQ(msg1_from_queue->content.text, "test1");
}