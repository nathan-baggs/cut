export module cut;

import std;
import std.compat;

namespace cut
{
export auto hello() -> std::string_view
{
    return "hello world";
}
}
