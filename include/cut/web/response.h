#pragma once

#include <cstdint>
#include <string>

#include "utils/json.h"

namespace cut::web
{

struct Response
{
    std::uint32_t code;
    std::string response;
};

constexpr auto Ok() -> Response
{
    return {.code = 200, .response = ""};
}

template <class T>
constexpr auto Ok(T &&obj) -> Response
{
    return {.code = 200, .response = utils::to_json<T>(obj)};
}

inline constexpr auto code_to_str(const Response &response) -> std::string
{
    switch (response.code)
    {
        case 200: return "OK";
    }

    return "unknown";
}

}
