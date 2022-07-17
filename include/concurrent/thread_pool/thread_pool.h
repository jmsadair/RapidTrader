#ifndef RAPID_TRADER_THREAD_POOL_H
#define RAPID_TRADER_THREAD_POOL_H
#include <thread>
#include <vector>
#include <functional>
#include <atomic>
#include "concurrent/thread_pool/queue.h"
#include "concurrent/thread_pool/task.h"

namespace Concurrent {
class ThreadPool
{
public:
    /**
     * A constructor for the thread pool.
     *
     * @param num_threads_ the number of worker threads that the thread pool will
     *                     spawn, require that num_threads_ is at least one.
     */
    explicit ThreadPool(uint8_t num_threads_ = std::thread::hardware_concurrency());

    /**
     * Submits a new task to the thread pool.
     *
     * @param task the task to submit.
     * @param queue_id the ID of the queue to submit the task to.
     */
    void submitTask(const Task &task, uint8_t queue_id);

    ~ThreadPool();

private:
    /**
     * Waits for and executes incoming tasks while running is true.
     *
     * @param queue_id the queue ID that corresponds to the queue that the worker
     *                 thread should draw tasks from, require that 0 <= queue_id < num_threads.
     */
    void workerThread(uint8_t queue_id);

    // The number of worker threads in the thread pool - must be at least 1.
    uint8_t num_threads;
    // Indicates whether the working threads are running.
    std::atomic_bool running;
    // The worker threads and their corresponding queues - each thread gets its own queue.
    std::vector<std::thread> threads;
    std::vector<Queue<Task>> thread_queues;
};
} // namespace Concurrent
#endif // RAPID_TRADER_THREAD_POOL_H
