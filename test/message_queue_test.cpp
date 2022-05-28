#include <gtest/gtest.h>
#include <future>
#include "message_queue.h"

// A simple message ADT for testing purposes.
struct TestMessage {
    std::string text;
};

TEST(MessageQueueTest, QueueShouldBlockWhenEmpty) {
    Messaging::MessageQueue queue;
    TestMessage msg1 {"test1"};
    TestMessage msg2 {"test2"};
    // The queue is empty - no messages can be popped until messages are pushed onto the queue.
    std::future<std::shared_ptr<Messaging::Message>> msg1_future =
            std::async(&Messaging::MessageQueue::waitAndPop, &queue);
    std::future<std::shared_ptr<Messaging::Message>> msg2_future =
            std::async(&Messaging::MessageQueue::waitAndPop, &queue);
    queue.push(msg1);
    queue.push(msg2);
    // Messages have been added to the queue - calling get() on the future should return a pointer to msg1.
    auto msg1_ptr = msg1_future.get();
    auto msg1_from_queue = dynamic_cast<Messaging::WrappedMessage<TestMessage>*>(msg1_ptr.get());
    // Casting should not return a null pointer.
    ASSERT_NE(msg1_from_queue, nullptr);
    // Messages have been added to the queue - calling get() on the future should return a pointer to msg2.
    auto msg2_ptr = msg2_future.get();
    auto msg2_from_queue = dynamic_cast<Messaging::WrappedMessage<TestMessage>*>(msg2_ptr.get());
    // Casting should not return a null pointer.
    ASSERT_NE(msg2_from_queue, nullptr);
    ASSERT_EQ(msg1_from_queue->content.text, "test1");
    ASSERT_EQ(msg2_from_queue->content.text, "test2");
}