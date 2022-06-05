#include <gtest/gtest.h>
#include <future>
#include <thread>
#include "receiver.h"

// Messages for testing.
struct AddMessage {};
struct SubtractMessage {};

struct MessagingTester {
    Messaging::Receiver receiver;
    int num = 0;
    void start() {
        try {
            while (true) {
                receiver.wait()
                        .handle<AddMessage>([&](const AddMessage &msg) { ++num; })
                        .handle<SubtractMessage>([&](const SubtractMessage &msg) { --num; });
            }
        } catch(const Messaging::CloseQueue&) {}
    }
};

TEST(MessagingTest, SenderAndRecieverShouldWork) {
    MessagingTester tester;
    auto sender = static_cast<Messaging::Sender>(tester.receiver);
    std::thread t1 {&MessagingTester::start, &tester};
    // Initialize some message for testing.
    AddMessage test_msg1;
    SubtractMessage test_msg2;
    // Expected value after sending the first message.
    int expected_num1 = 2;
    // Add 3 to num.
    sender.send(test_msg1);
    sender.send(test_msg1);
    sender.send(test_msg1);
    // Subtract 1 from num.
    sender.send(test_msg2);
    sender.send(Messaging::CloseQueue());
    t1.join();
    ASSERT_EQ(tester.num, expected_num1);
}