#pragma once

#include <csignal>
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

        handle_route("Get", "MyController", "get");
        handle_route("Get", "AnotherController", "users");
        handle_route("Get", "AnotherController", "users2");

        std::println("running");
    }

  private:
    auto handle_route(std::string_view method, std::string_view controller, std::string_view route)
    {
        std::println("handling {} {} {}", method, controller, route);

        auto handled = false;

        details::Visitor<Controllers...>::visit(
            controllers_,
            [&handled, method, controller_name = controller, route](auto &controller)
            {
                if (controller.name() == controller_name)
                {
                    handled |= controller.dispatch_handler(method, route);
                }
            });

        if (!handled)
        {
            throw std::runtime_error(std::format("failed to handle request: {} {} {}", method, controller, route));
        }
    }

    std::tuple<Controllers...> controllers_;
};

}
