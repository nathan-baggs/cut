#pragma once

#include <experimental/meta>

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
};

}
