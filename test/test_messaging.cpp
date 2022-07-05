#include <gtest/gtest.h>
#include <future>
#include <thread>
#include "receiver.h"

// Messages for testing.
struct Message1
{};
struct Message2
{};

struct MessagingTester
{
    Messaging::Receiver receiver;
    int num = 0;
    void start()
    {
        try
        {
            while (true)
            {
                receiver.wait().handle<Message1>([&](const Message1 &msg) { ++num; }).handle<Message2>([&](const Message2 &msg) { --num; });
            }
        }
        catch (const Messaging::CloseQueue &)
        {}
    }
};

TEST(MessagingTest, SenderAndRecieverShouldWork)
{
    MessagingTester tester;
    auto sender = static_cast<Messaging::Sender>(tester.receiver);

    // Spawn a thread for handling incoming messages.
    std::thread t1{&MessagingTester::start, &tester};

    // Initialize some message for testing.
    Message1 test_msg1;
    Message2 test_msg2;

    // Send a message that increments num.
    sender.send(test_msg1);

    // Send a message that decrements num.
    sender.send(test_msg2);

    // Close out the message queue and join the thread.
    sender.send(Messaging::CloseQueue());
    t1.join();

    // Number should be 0.
    ASSERT_EQ(tester.num, 0);
}