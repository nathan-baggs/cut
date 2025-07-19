#include <iostream>
#include <print>

#include "cut/web/annotations.h"
#include "cut/web/app.h"
#include "cut/web/controller_base.h"

// clang-format off
class MyController : public cut::web::ControllerBase
{
  public:
    ~MyController() override = default;

    [[=cut::web::Get]]
    auto get() -> void
    {
        std::println("MyController get called");
    }

    auto another_method() -> void
    {
    }
};

class AnotherController : public cut::web::ControllerBase
{
  public:
    ~AnotherController() override = default;

    [[=cut::web::Get]]
    auto users() -> void
    {
        std::println("AnotherController users called");
    }
};
// clang-format on

auto main() -> int
{
    try
    {
        auto app = cut::web::App<MyController, AnotherController>{};

        app.run();
    }
    catch (std::runtime_error &e)
    {
        std::println(std::cerr, "{}", e.what());
    }

    return 0;
}
