#pragma once

#include <experimental/meta>
#include <print>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

namespace cut::utils
{

namespace details
{

template <class T>
auto add_json_member(const T &obj, std::string_view key, ::nlohmann::json &json)
{
    if constexpr (^^T == ^^int)
    {
        json[key] = obj;
    }
    else if constexpr (^^T == std::meta::dealias(^^std::string))
    {
        json[key] = obj;
    }
    else
    {
        auto &next_key = json[key];

        constexpr auto ctx = std::meta::access_context::current();
        template for (constexpr auto member : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx)))
        {
            add_json_member(obj.[:member:], std::meta::display_string_of(member), next_key);
        }
    }
}
}

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

template <class T>
auto to_json([[maybe_unused]] const T &obj) -> std::string
{
    auto json = ::nlohmann::json{};

    constexpr auto ctx = std::meta::access_context::current();
    template for ([[maybe_unused]] constexpr auto member :
                  std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx)))
    {
        details::add_json_member(obj.[:member:], std::meta::display_string_of(member), json);
    }

    return json.dump();
}

}
