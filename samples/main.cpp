#include <print>

#include "cut/web/app.h"
#include "cut/web/controller_base.h"

class MyController : public cut::web::ControllerBase
{
  public:
    ~MyController() override = default;

    auto get() -> void
    {
        std::println("get called");
    }
};

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
