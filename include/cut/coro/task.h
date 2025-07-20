#pragma once

#include <coroutine>
#include <future>
#include <stdexcept>

#include "utils/log.h"

namespace cut::coro
{

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

        auto return_void()
        {
        }

        auto get_return_object()
        {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        auto unhandled_exception()
        {
            exception_ptr = std::current_exception();
        }

        std::exception_ptr exception_ptr = nullptr;
    };

    ~Task()
    {
        utils::log::debug("~Task()");

        if (handle_)
        {
            handle_.destroy();
        }
    }

    Task(const Task &) = delete;
    auto operator=(const Task &) -> Task & = delete;
    Task(Task &&other)
        : handle_(std::exchange(other.handle_, nullptr))
    {
    }

    auto operator=(Task &&other) -> Task &
    {
        std::ranges::swap(handle_, other.handle_);
        return *this;
    }

    auto resume() -> void
    {
        handle_.resume();

        if (handle_ && handle_.promise().exception_ptr)
        {
            std::rethrow_exception(handle_.promise().exception_ptr);
        }
    }

    auto native_handle() const -> std::coroutine_handle<>
    {
        return handle_;
    }

  private:
    explicit Task(std::coroutine_handle<promise_type> handle)
        : handle_(handle)
    {
    }

    std::coroutine_handle<promise_type> handle_;
};

}
