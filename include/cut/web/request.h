#pragma once

#include <format>
#include <map>
#include <optional>
#include <string>

namespace cut::web
{

struct Request
{
    std::string method;
    std::string controller;
    std::string route;
    std::map<std::string, std::string> headers;
    std::optional<std::string> body;
};

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
