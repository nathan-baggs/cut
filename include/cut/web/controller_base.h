#pragma once

#include <experimental/meta>
#include <format>
#include <optional>
#include <stdexcept>
#include <string_view>

#include "utils/json.h"
#include "web/request.h"
#include "web/response.h"

using namespace std::literals;

namespace cut::web
{

class ControllerBase
{
  public:
    virtual ~ControllerBase() = default;

    template <class Self>
    consteval auto name(this Self &&)
    {
        using T = std::decay_t<Self>;
        static_assert(sizeof(T) > 0);
        return std::meta::display_string_of(std::meta::dealias(^^T));
    }

    template <class Self>
    auto dispatch_handler(this Self &&self, const Request &request) -> std::optional<Response>
    {
        using T = std::decay_t<Self>;
        static_assert(sizeof(T) > 0);

        constexpr auto ctx = std::meta::access_context::current();
        template for (constexpr auto member :
                      std::define_static_array(std::meta::members_of(std::meta::dealias(^^T), ctx)))
        {
            if constexpr (
                std::meta::is_function(member) && !std::meta::is_special_member_function(member) &&
                !std::meta::is_constructor(member) && !std::meta::is_default_constructor(member))
            {
                if (request.route == std::meta::display_string_of(member))
                {
                    constexpr auto annotations = std::define_static_array(std::meta::annotations_of(member));

                    if constexpr (!std::ranges::empty(annotations))
                    {
                        template for (constexpr auto annotation : annotations)
                        {
                            using A = typename[:std::meta::type_of(annotation):];
                            if (A::method == request.method)
                            {
                                constexpr auto params = std::define_static_array(std::meta::parameters_of(member));
                                if constexpr (std::ranges::size(params) == 1)
                                {
                                    if (!request.body)
                                    {
                                        throw std::runtime_error(std::format("missing body for {}", request));
                                    }

                                    auto arg =
                                        utils::from_json<typename[:std::meta::type_of(params.front()):]>(*request.body);
                                    return {self.[:member:](std::move(arg)).native_handle().promise().value};
                                }
                                else
                                {
                                    return {self.[:member:]().native_handle().promise().value};
                                }
                            }
                        }
                    }
                }
            }
        }

        return std::nullopt;
    }
};

}
