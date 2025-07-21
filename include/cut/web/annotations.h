#pragma once

#include <string_view>

using namespace std::literals;

namespace cut::web
{

namespace annotations
{

struct Get
{
    static constexpr auto method = "GET"sv;
};

struct Post
{
    static constexpr auto method = "POST"sv;
};

}

static constexpr auto Get = annotations::Get{};
static constexpr auto Post = annotations::Post{};

}
