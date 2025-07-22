#pragma once

#include <format>
#include <map>
#include <optional>
#include <string>

namespace cut::web
{

/**
 * Represents a client HTTP request.
 */
struct Request
{
    /** The HTTP method (will always be upper case). */
    std::string method;

    /** The controller the request is for i.e. the first part of the url. */
    std::string controller;

    /** The route the request is for i.e. the second part of the url. */
    std::string route;

    /** Map of headers. Id use unordered_map but my build of clang is unhappy with std::hash... */
    std::map<std::string, std::string> headers;

    /** Optional body - assumed the be JSON if present. */
    std::optional<std::string> body;
};

/**
 * Format a request for printing.
 *
 * @param request
 *   The request to format.
 *
 * @returns
 *   The Request as a string.
 */
inline auto to_string(const Request &request)
{
    return std::format(
        "{}\n{}/{}\b{}\n{}",
        request.method,
        request.controller,
        request.route,
        request.headers,
        request.body ? *request.body : "<no body>");
}

}
