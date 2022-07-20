#ifndef RAPID_TRADER_THREAD_POOL_H
#define RAPID_TRADER_THREAD_POOL_H
#include <thread>
#include <vector>
#include <atomic>
#include "concurrent/thread_pool/thread_joiner.h"
#include "concurrent/thread_pool/queue.h"

namespace Concurrent{
class ThreadPool
{
public:
    /**
     * A constructor for the thread pool.
     *
     * @param num_threads_ the number of worker threads that will be spawned by
     *                     the thread pool, require that num_threads_ is positive.
     */
    explicit ThreadPool(uint8_t num_threads_ = std::thread::hardware_concurrency())
        : num_threads(num_threads_)
        , running(true)
        , thread_joiner(threads)
    {
        assert(num_threads > 0 && "Thread pool requires at least one thread!");
        thread_queues.reserve(num_threads);
        threads.reserve(num_threads);
        // Try to spawn the threads.
        try
        {
            for (uint8_t i = 0; i < num_threads; ++i)
            {
                thread_queues.push_back(std::make_unique<Queue<std::function<void()>>>());
                threads.emplace_back(&ThreadPool::workerThread, this, i);
            }
        }
        catch (...)
        {
            running = false;
            throw;
        }
    }

    /**
     * Submit a new task to the thread pool.
     *
     * @tparam F the type of the function that is being submitted.
     * @param queue_id the id of the queue that the task will be submitted to,
     *                 require that 0 <= queue_id < num_threads.
     * @param f the task being submitted.
     */
    template<typename F>
    void submitTask(uint8_t queue_id, F f)
    {
        thread_queues[queue_id]->push(std::function<void()>(f));
    }

    /**
     * @return number of worker threads the thread pool is using.
     */
    [[nodiscard]] uint8_t numberOfThreads() const
    {
        return num_threads;
    }

    /**
     * A destructor for the thread pool. Notifies worker threads that they are finished.
     */
    ~ThreadPool()
    {
        running = false;
    };

private:

    /**
     * Wait for and execute incoming tasks.
     *
     * @param queue_id the queue ID that the worker thread should
     *                 draw tasks from, require that 0 <= queue < num_threads.
     */
    void workerThread(uint8_t queue_id)
    {
        assert(queue_id < thread_queues.size() && "Invalid queue ID!");
        while (running)
        {
            std::function<void()> task;
            if (thread_queues[queue_id]->tryPop(task))
                task();
            else
                std::this_thread::yield();
        }
    }

    // IMPORTANT: The running flag and the work queues must be declared
    // before the vector containing the threads. Otherwise, the queue and
    // the flag would be destroyed before the threads have finished their work.

    // Indicates whether the working threads are running.
    std::atomic_bool running;
    // The number of worker threads in the thread pool - must be at least 1.
    uint8_t num_threads;
    // Thread-safe queue for each worker thread.
    std::vector<std::unique_ptr<Queue<std::function<void()>>>> thread_queues;
    // The worker threads and their corresponding queues - each thread gets its own queue.
    std::vector<std::thread> threads;
    // Handles cleaning up the worker threads when the thread pool is destroyed.
    ThreadJoiner thread_joiner;
};
} // namespace Concurrent
#endif // RAPID_TRADER_THREAD_POOL_H
