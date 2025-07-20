#pragma once

#include <filesystem>
#include <format>
#include <print>
#include <source_location>

namespace cut::utils::log
{

enum class Level
{
    DEBUG,
    INFO,
    WARN,
    ERR
};

template <Level L, class... Args>
struct Print
{
    Print(std::format_string<Args...> msg, Args &&...args, std::source_location loc = std::source_location::current())
    {
        auto c = '?';
        if constexpr (L == Level::DEBUG)
        {
            c = 'D';
        }
        else if constexpr (L == Level::INFO)
        {
            c = 'I';
        }
        else if constexpr (L == Level::WARN)
        {
            c = 'W';
        }
        else if constexpr (L == Level::ERR)
        {
            c = 'E';
        }

        const auto path = std::filesystem::path{loc.file_name()};

        const auto log_line = std::format(
            "[{}] {}:{} {}", c, path.filename().string(), loc.line(), std::format(msg, std::forward<Args>(args)...));

        std::println("{}", log_line);
    }
};

template <Level L = {}, class... Args>
Print(std::format_string<Args...>, Args &&...) -> Print<L, Args...>;

template <class... Args>
using debug = Print<Level::DEBUG, Args...>;

template <class... Args>
using info = Print<Level::INFO, Args...>;

template <class... Args>
using warn = Print<Level::WARN, Args...>;

template <class... Args>
using error = Print<Level::ERR, Args...>;

}
