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
        std::println("get called");
    }
};
// clang-format on

class AnotherController : public cut::web::ControllerBase
{
  public:
    ~AnotherController() override = default;
};

auto main() -> int
{
    auto app = cut::web::App<MyController, AnotherController>{};

    app.run();

    return 0;
}
