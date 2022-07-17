#ifndef RAPID_TRADER_TASK_H
#define RAPID_TRADER_TASK_H
#include <functional>
namespace Concurrent {
/**
 * A generic task that will be submitted to and executed by the thread pool.
 */
struct Task
{
    Task() = default;

    template<typename Function, typename... Args>
    explicit Task(Function f, Args... args)
        : callback{std::bind(f, args...)}
    {}

    void operator()() const
    {
        callback();
    }

private:
    std::function<void()> callback;
};
} // namespace Concurrent
#endif // RAPID_TRADER_TASK_H
