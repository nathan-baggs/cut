#pragma once

#include <experimental/meta>
#include <map>
#include <string>
#include <vector>

#include "utils/log.h"
#include "utils/type_list.h"
#include "web/response.h"

namespace cut::web
{

namespace details
{
struct Info
{
    std::string title;
    std::string version;
    std::string description;
};

struct Server
{
    std::string url;
};

struct Method
{
    std::string summary;
    std::string operationId;
};

struct Path
{
    std::map<std::string, Method> methods;
};

struct OpenAPI
{
    std::string openapi;
    Info info;
    std::vector<Server> servers;
    // clang-format off
    [[=utils::IncludeMemberName]]
    std::map<std::string, Path> paths;
    // clang-format on
};

template <class Controller>
auto parse_paths(std::map<std::string, Path> &paths)
{
    constexpr auto ctx = std::meta::access_context::current();
    template for (constexpr auto member :
                  std::define_static_array(std::meta::members_of(std::meta::dealias(^^Controller), ctx)))
    {
        if constexpr (
            std::meta::is_function(member) && !std::meta::is_special_member_function(member) &&
            !std::meta::is_constructor(member) && !std::meta::is_default_constructor(member))
        {
            constexpr auto annotations = std::define_static_array(std::meta::annotations_of(member));

            if constexpr (!std::ranges::empty(annotations))
            {
                template for (constexpr auto annotation : annotations)
                {
                    using A = typename[:std::meta::type_of(annotation):];

                    paths[std::format(
                              "/{}/{}",
                              std::meta::display_string_of(^^Controller),
                              std::meta::display_string_of(member))]
                        .methods["get"] = Method{
                        .summary = std::string(std::meta::display_string_of(member)),
                        .operationId = std::string(std::meta::display_string_of(member))};
                }
            }
        }
    }
}

}

template <class... Controllers>
inline auto json_api() -> Response
{
    utils::log::info("handling /api/json");

    auto open_api = details::OpenAPI{
        .openapi = "3.0.0",
        .info = {.title = "cut app", .version = "0.0.1", .description = "reflection based web app"},
        .servers = {{.url = "http://localhost:6375"}},
        .paths = {}};

    (parse_paths<Controllers>(open_api.paths), ...);

    return Ok(std::move(open_api));
}
}
