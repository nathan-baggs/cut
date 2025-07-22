#pragma once

#include <coroutine>
#include <exception>

#include "utils/log.h"

namespace cut::coro
{

/**
 * Basic coroutine task that starts unsuspended and ends suspended. Can return a value (of type T).
 */
template <class T>
class Task
{
  public:
    struct promise_type
    {
        auto initial_suspend()
        {
            return std::suspend_never{};
        }

        auto final_suspend() noexcept
        {
            return std::suspend_always{};
        }

        auto return_value(T &&value)
        {
            if (exception_ptr)
            {
                std::rethrow_exception(exception_ptr);
            }

            this->value = std::move(value);
        }

        auto get_return_object()
        {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        auto unhandled_exception()
        {
            utils::log::warn("unhandled exception");
            exception_ptr = std::current_exception();
        }

        std::exception_ptr exception_ptr = nullptr;
        T value;
    };

    auto native_handle() -> std::coroutine_handle<promise_type>
    {
        return handle_;
    }

    ~Task()
    {
        if (handle_)
        {
            handle_.destroy();
        }
    }

  private:
    explicit Task(std::coroutine_handle<promise_type> handle)
        : handle_(handle)
    {
    }

    std::coroutine_handle<promise_type> handle_;
};
}
