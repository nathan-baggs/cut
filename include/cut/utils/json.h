#pragma once

#include <experimental/meta>
#include <string_view>

#include <nlohmann/json.hpp>

namespace cut::utils
{

template <class T>
auto from_json(std::string_view json_str) -> T
{
    auto res = T{};

    const auto json = ::nlohmann::json::parse(json_str);

    constexpr auto ctx = std::meta::access_context::current();
    template for (constexpr auto member : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx)))
    {
        if (const auto value = json.find(std::meta::display_string_of(member)); value != std::end(json))
        {
            res.[:member:] = *value;
        }
    }

    return res;
}

}
