#pragma once

#include <coroutine>
#include <exception>

namespace cut::coro
{

class VoidTask
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
            return std::suspend_never{};
        }

        auto return_void()
        {
        }

        auto get_return_object()
        {
            return VoidTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        auto unhandled_exception()
        {
            exception_ptr = std::current_exception();
        }

        std::exception_ptr exception_ptr = nullptr;
    };

    auto native_handle() -> std::coroutine_handle<promise_type>
    {
        return handle_;
    }

  private:
    explicit VoidTask(std::coroutine_handle<promise_type> handle)
        : handle_(handle)
    {
    }

    std::coroutine_handle<promise_type> handle_;
};

}
