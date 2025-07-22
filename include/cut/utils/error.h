#pragma once

#include <exception>
#include <format>
#include <memory>
#include <stdexcept>
#include <utility>

#include "utils/auto_release.h"
#include "utils/log.h"

namespace cut::utils
{

template <class... Args>
constexpr auto expect(bool predicate, std::format_string<Args...> msg, Args &&...args) -> void
{
    if (!predicate)
    {
        log::error("{}", std::format(msg, std::forward<Args>(args)...));
        // no stacktrack on clang yet :(
        // log::error("{}", std::stacktrace::current(1));
        std::terminate();
        std::unreachable();
    }
}

template <class T, class... Args>
constexpr auto expect(std::unique_ptr<T> &obj, std::format_string<Args...>(msg), Args &&...args) -> void
{
    expect(!!obj, msg, std::forward<Args>(args)...);
}

template <class... Args>
auto ensure(bool predicate, std::format_string<Args...> msg, Args &&...args) -> void
{
    if (!predicate)
    {
        log::error("{}", std::format(msg, std::forward<Args>(args)...));
        throw std::runtime_error(std::format(msg, std::forward<Args>(args)...));
    }
}

template <class T, T Invalid, class... Args>
auto ensure(AutoRelease<T, Invalid> &obj, std::format_string<Args...> msg, Args &&...args) -> void
{
    ensure(!!obj, msg, std::forward<Args>(args)...);
}

template <class T, class D, class... Args>
auto ensure(std::unique_ptr<T, D> &obj, std::format_string<Args...> msg, Args &&...args) -> void
{
    ensure(!!obj, msg, std::forward<Args>(args)...);
}

}
