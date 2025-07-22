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

// bunch of structs to mimic the OpenAPI format

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

/**
 * Helper function to fill out the above structs for a Controller. Will reflect through the controller to find the
 * members which are annotated to handle requests.
 *
 * @param paths
 *   The paths map to fill.
 */
template <class Controller>
auto parse_paths(std::map<std::string, Path> &paths)
{
    constexpr auto ctx = std::meta::access_context::current();

    // get each member of the class
    template for (constexpr auto member :
                  std::define_static_array(std::meta::members_of(std::meta::dealias(^^Controller), ctx)))
    {
        // filter out things that cannot be a member
        if constexpr (
            std::meta::is_function(member) && !std::meta::is_special_member_function(member) &&
            !std::meta::is_constructor(member) && !std::meta::is_default_constructor(member))
        {
            constexpr auto annotations = std::define_static_array(std::meta::annotations_of(member));

            // only annotated functions could be handlers
            if constexpr (!std::ranges::empty(annotations))
            {
                // lazy - assuming all annotations are request ones, this will break if any other annotation type is
                // used
                template for (constexpr auto annotation : annotations)
                {
                    // get the method string (as lower case)
                    using A = typename[:std::meta::type_of(annotation):];
                    const auto method = A::method | std::views::transform([](auto e) { return std::tolower(e); }) |
                                        std::ranges::to<std::string>();

                    // fill out the struct
                    paths[std::format(
                              "/{}/{}",
                              std::meta::display_string_of(^^Controller),
                              std::meta::display_string_of(member))]
                        .methods[method] = Method{
                        .summary = std::string(std::meta::display_string_of(member)),
                        .operationId = std::string(std::meta::display_string_of(member))};
                }
            }
        }
    }
}

}

/**
 * Create a JSON string conforming to the OpenAPI standard that details all the available endpoints in all the
 * controllers. This can be shoved into swagger.
 *
 * @returns
 *   A Response object that can be returned to a client request the JSON.
 */
template <class... Controllers>
inline auto json_api() -> Response
{
    utils::log::info("handling /api/json");

    // boilerplate, should be configurable but it's not
    auto open_api = details::OpenAPI{
        .openapi = "3.0.0",
        .info = {.title = "cut app", .version = "0.0.1", .description = "reflection based web app"},
        .servers = {{.url = "http://localhost:6375"}},
        .paths = {}};

    // fold the controllers over the method parse function
    (parse_paths<Controllers>(open_api.paths), ...);

    return Ok(std::move(open_api));
}
}
