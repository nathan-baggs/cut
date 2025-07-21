#pragma once

#include <functional>

namespace cut::db
{

class Db
{
  public:
    struct Awaitable
    {
        bool await_ready()
        {
            return true;
        }

        bool await_suspend(std::coroutine_handle<>)
        {
            return true;
        }

        auto await_resume()
        {
            return get_value();
        }

        std::function<int()> get_value;
    };

    virtual ~Db() = default;

    virtual auto test() -> Awaitable = 0;
};

}
