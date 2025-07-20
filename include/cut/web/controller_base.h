#pragma once

#include <experimental/meta>
#include <optional>
#include <string_view>

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
        using t = std::decay_t<Self>;
        static_assert(sizeof(t) > 0);
        return std::meta::display_string_of(std::meta::dealias(^^t));
    }

    template <class Self>
    auto dispatch_handler(this Self &&self, std::string_view method, std::string_view route) -> std::optional<Response>
    {
        using t = std::decay_t<Self>;
        static_assert(sizeof(t) > 0);

        constexpr auto ctx = std::meta::access_context::current();
        template for (constexpr auto info :
                      std::define_static_array(std::meta::members_of(std::meta::dealias(^^t), ctx)))
        {
            if constexpr (
                std::meta::is_function(info) && !std::meta::is_special_member_function(info) &&
                !std::meta::is_constructor(info) && !std::meta::is_default_constructor(info))
            {
                if (route == std::meta::display_string_of(info))
                {
                    constexpr auto annotations = std::define_static_array(std::meta::annotations_of(info));

                    if constexpr (!std::ranges::empty(annotations))
                    {
                        template for (constexpr auto annotation : annotations)
                        {
                            using A = typename[:std::meta::type_of(annotation):];

                            if (A::method == method)
                            {
                                return {self.[:info:]().native_handle().promise().value};
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
