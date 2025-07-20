#pragma once

#include <cstdint>
#include <string>

namespace cut::web
{

struct Response
{
    std::uint32_t code;
    std::string response;
};

inline constexpr auto Ok() -> Response
{
    return {.code = 200, .response = {}};
}

inline constexpr auto Ok(std::string response) -> Response
{
    return {.code = 200, .response = std::move(response)};
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
