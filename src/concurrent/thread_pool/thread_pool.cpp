#include <cassert>
#include "concurrent/thread_pool/thread_pool.h"

Concurrent::ThreadPool::ThreadPool(uint8_t num_threads_)
    : num_threads(num_threads_)
    , running(true)
{
    assert(num_threads > 0 && "Thread pool requires at least one thread!");
    threads.reserve(num_threads);
    thread_queues.resize(num_threads);
    for (uint8_t i = 0; i < num_threads; ++i)
        threads.emplace_back(&ThreadPool::workerThread, this, i);
}

Concurrent::ThreadPool::~ThreadPool()
{
    running.store(false);
    for (auto &thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }
}

void Concurrent::ThreadPool::submitTask(const Concurrent::Task &task, uint8_t queue_id)
{
    assert(queue_id < thread_queues.size() && "Invalid queue ID!");
    thread_queues[queue_id].push(task);
}

void Concurrent::ThreadPool::workerThread(uint8_t queue_id)
{
    assert(queue_id < thread_queues.size() && "Invalid queue ID!");
    Queue<Task> &thread_queue = thread_queues[queue_id];
    Task task;
    while (running.load())
    {
        if (thread_queue.tryPop(task))
            task();
        else
            std::this_thread::yield();
    }
}
