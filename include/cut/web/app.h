#pragma once

#include <cstddef>
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>
#include <utility>

using namespace std::literals;

namespace cut::web
{

namespace details
{

template <class... Controllers>
struct Visitor
{
    template <class F>
    static constexpr auto visit(std::tuple<Controllers...> &controllers, F &&f)
    {
        template for (constexpr auto i : std::views::iota(0zu, sizeof...(Controllers)))
        {
            auto controller = std::get<i>(controllers);
            f(controller);
        }
    }
};

}

template <class... Controllers>
class App
{
  public:
    App() = default;
    ~App() = default;
    App(const App &) = delete;
    auto operator=(const App &) -> App & = delete;
    App(App &&) = delete;
    auto operator=(App &&) -> App & = delete;

    auto run()
    {
        details::Visitor<Controllers...>::visit(
            controllers_, [](auto &controller) { std::println("{} registered", controller.name()); });

        std::println("simulating get");

        details::Visitor<Controllers...>::visit(
            controllers_,
            [](auto &controller)
            {
                if constexpr (controller.name() == "MyController"sv)
                {
                    controller.get();
                }
            });

        std::println("running");
    }

  private:
    std::tuple<Controllers...> controllers_;
};

}
