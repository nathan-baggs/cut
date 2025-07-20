#pragma once

#include <csignal>
#include <cstddef>
#include <print>
#include <ranges>
#include <string_view>
#include <sys/socket.h>
#include <tuple>
#include <utility>

#include "coro/event_loop.h"
#include "coro/task.h"
#include "utils/log.h"
#include "web/server_socket.h"

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
            controllers_, [](auto &controller) { utils::log::info("{} registered", controller.name()); });

        utils::log::info("running");

        auto ev = coro::EventLoop{};
        auto server_socket = ServerSocket{6375, ev};

        const auto task = accept(server_socket);
        ev.run();
    }

  private:
    auto handle_route(std::string_view method, std::string_view controller, std::string_view route)
    {
        utils::log::info("handling {} {} {}", method, controller, route);

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

    auto accept(ServerSocket &server_socket) -> coro::Task
    {
        utils::log::info("starting server loop");

        for (;;)
        {
            const auto client = co_await server_socket.accept();
            utils::log::info("client connected {}", client);
        }
    }

    std::tuple<Controllers...> controllers_;
};

}
