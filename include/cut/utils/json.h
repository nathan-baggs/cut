#pragma once

#include <cinttypes>
#include <experimental/meta>
#include <map>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

namespace cut::utils
{

namespace annotations
{
struct IncludeMemberName
{
};
}

static constexpr auto IncludeMemberName = annotations::IncludeMemberName{};

namespace details
{

template <class T>
struct JsonMember
{
    template <class U>
    static constexpr auto add(const T &obj, U key, ::nlohmann::json &json, bool = false) -> void
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
            template for (constexpr auto member :
                          std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx)))
            {
                JsonMember<typename[:std::meta::type_of(member):]>::add(
                    obj.[:member:], std::meta::display_string_of(member), next_key);
            }
        }
    }
};

template <class T>
struct JsonMember<std::vector<T>>
{
    static constexpr auto add(const std::vector<T> &obj, std::string_view key, ::nlohmann::json &json, bool = false)
        -> void
    {
        auto &next_key = json[key];
        for (auto i = 0u; i < obj.size(); ++i)
        {
            JsonMember<T>::add(obj[i], i, next_key);
        }
    }
};

template <class K, class V>
struct JsonMember<std::map<K, V>>
{
    static constexpr auto add(
        const std::map<K, V> &obj,
        std::string_view key,
        ::nlohmann::json &json,
        bool include_member_name = false) -> void
    {
        for (const auto &[k, v] : obj)
        {
            std::println("k: {} {}", k, include_member_name);
            JsonMember<V>::add(v, k, include_member_name ? json[key] : json);
        }
    }
};
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
auto to_json(const T &obj) -> std::string
{
    auto json = ::nlohmann::json{};

    constexpr auto ctx = std::meta::access_context::current();
    template for ([[maybe_unused]] constexpr auto member :
                  std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx)))
    {
        constexpr auto annotations = std::define_static_array(std::meta::annotations_of(member));
        constexpr auto include_member_name = std::ranges::size(annotations) == 1;
        details::JsonMember<typename[:std::meta::type_of(member):]>::add(
            obj.[:member:], std::meta::display_string_of(member), json, include_member_name);
    }

    return json.dump();
}
}
