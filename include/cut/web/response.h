#pragma once

#include <concepts>
#include <cstdint>
#include <sstream>
#include <string>

#include "utils/json.h"

namespace cut::web
{

namespace details
{
template <class T>
concept StringIsh = std::convertible_to<T, std::string>;
}

/**
 * Represents a HTTP response to the client.
 */
struct Response
{
    /** The HTTP status code. */
    std::uint32_t code;

    /** The response body - Assumed to be plain text or serialised JSON. */
    std::string response;
};

// helper functions to produce responses for specific status codes

constexpr auto Ok() -> Response
{
    return {.code = 200, .response = ""};
}

template <class T>
constexpr auto Ok(T &&obj) -> Response
{
    if constexpr (details::StringIsh<T>)
    {
        return {.code = 200, .response = obj};
    }
    else
    {
        return {.code = 200, .response = utils::to_json<T>(obj)};
    }
}

constexpr auto Created() -> Response
{
    return {.code = 201, .response = ""};
}

template <class T>
constexpr auto Created(T &&obj) -> Response
{
    if constexpr (details::StringIsh<T>)
    {
        return {.code = 201, .response = obj};
    }
    else
    {
        return {.code = 201, .response = utils::to_json<T>(obj)};
    }
}

constexpr auto NotFound(const std::string &obj) -> Response
{
    return {.code = 404, .response = obj};
}

/** Helper function to convert a HTTP response to human readable string.
 *
 * @param response
 *   The response to format.
 *
 * @returns
 *   The HTTP code as a string.
 */
inline constexpr auto code_to_str(const Response &response) -> std::string
{
    switch (response.code)
    {
        case 200: return "OK";
        case 201: return "CREATED";
        case 303: return "NOT FOUND";
    }

    return "unknown";
}

/**
 * Format a response into a string that can be sent back.
 *
 * @param response
 *   The response to format
 *
 * @returns
 *   The response as a formatted string for sending.
 */
auto format_response(const Response &response) -> std::string
{
    auto strm = std::stringstream{};
    strm << std::format("HTTP/1.1 {} {}\r\n", response.code, code_to_str(response));

    // add the magic "make it work" header
    strm << "Access-Control-Allow-Origin: *\r\n";

    strm << std::format("Content-Length: {}\r\n", response.response.length());
    strm << "\r\n";
    strm << response.response;

    return strm.str();
}

}
