#ifndef RAPID_TRADER_TASK_H
#define RAPID_TRADER_TASK_H
#include <memory>
namespace Concurrent {
class Task
{
public:
    Task(const Task &) = delete;
    Task(Task &) = delete;
    Task &operator=(const Task &) = delete;
    Task() = default;

    template<typename F>
    Task(F &&f_)
        : callable(new CallableType<F>(std::forward<F>(f_)))
    {}

    Task(Task &&other) noexcept
        : callable(std::move(other.callable))
    {}

    Task &operator=(Task &&other) noexcept
    {
        callable = std::move(other.callable);
        return *this;
    }

    void operator()()
    {
        callable->call();
    }

private:
    struct Callable
    {
        virtual void call() = 0;
        virtual ~Callable() = default;
    };
    std::unique_ptr<Callable> callable;

    template<typename F>
    struct CallableType : Callable
    {
        F f;
        CallableType(F &&f_)
            : f(std::move(f_))
        {}
        void call() override
        {
            f();
        }
    };
};
} // namespace Concurrent
#endif // RAPID_TRADER_TASK_H
