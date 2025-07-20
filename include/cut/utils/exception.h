#pragma once

#include <format>
#include <stacktrace>
#include <stdexcept>

namespace cut::utils
{

class Exception : public std::runtime_error
{
  public:
    template <class... Args>
    Exception(std::format_string<Args...> msg, Args &&...args)
        : std::runtime_error{std::format(msg, std::forward<Args>(args)...)}
        , trace_(std::stacktrace::current(1))
    {
    }

    auto what() const noexcept -> const char * override
    {
        return std::format("{}\n{}", std::runtime_error::what(), trace_);
    }

  private:
    std::stacktrace trace_;
};

}
