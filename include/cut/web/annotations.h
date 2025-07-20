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

}

static constexpr auto Get = annotations::Get{};

}
