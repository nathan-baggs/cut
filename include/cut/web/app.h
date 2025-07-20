#pragma once

#include <csignal>
#include <cstddef>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_set>
#include <utility>

#include <sys/socket.h>

#include "coro/event_loop.h"
#include "coro/void_task.h"
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

        accept(server_socket);
        ev.run();
    }

  private:
    auto handle_route(std::string_view method, std::string_view controller, std::string_view route) -> std::string
    {
        utils::log::debug("handling {} {} {}", method, controller, route);

        auto response_str = std::string{};

        details::Visitor<Controllers...>::visit(
            controllers_,
            [&response_str, method, controller_name = controller, route](auto &controller)
            {
                if (controller.name() == controller_name)
                {
                    if (const auto response = controller.dispatch_handler(method, route); response)
                    {
                        auto strm = std::stringstream{};
                        strm << std::format("HTTP/1.1 {} {}\r\n", response->code, code_to_str(*response));
                        strm << std::format("Content-Length: {}\r\n", response->response.length());
                        strm << "\r\n";
                        strm << response->response;

                        response_str = strm.str();
                    }
                }
            });

        if (response_str.empty())
        {
            throw std::runtime_error(std::format("failed to handle request: {} {} {}", method, controller, route));
        }

        return response_str;
    }

    auto read(std::unique_ptr<ClientSocket> client_socket) -> coro::VoidTask
    {
        utils::log::info("new client: {}", client_socket->native_handle());

        for (;;)
        {
            auto headers = std::vector<std::tuple<std::string, std::string>>{};

            const auto request_line = co_await client_socket->read_until("\r\n"sv);

            for (;;)
            {
                const auto header = co_await client_socket->read_until("\r\n"sv);
                const auto colon_index = header.find(':');

                if (header == "\r\n"sv)
                {
                    break;
                }

                if (colon_index == std::string::npos)
                {
                    throw std::runtime_error("invalid header");
                }

                headers.emplace_back(
                    header.substr(0u, colon_index),
                    header.substr(colon_index + 1u, header.length() - colon_index - 3u));
            }

            const auto request_line_parts = request_line | std::views::split(' ') | std::ranges::to<std::vector>();
            if (std::ranges::size(request_line_parts) != 3)
            {
                throw std::runtime_error("invalid request line");
            }

            const auto route_parts = request_line_parts[1] | std::views::split('/') | std::ranges::to<std::vector>();
            if (std::ranges::size(route_parts) != 3)
            {
                throw std::runtime_error("invalid request line (route)");
            }

            utils::log::info("request line: {}", request_line);
            utils::log::info("headers: {}", headers);

            const auto response_str = handle_route(
                std::string_view{request_line_parts[0]},
                std::string_view{route_parts[1]},
                std::string_view{route_parts[2]});

            utils::log::info("sending response: {}", response_str);

            client_socket->write(response_str);
        }
    }

    auto accept(ServerSocket &server_socket) -> coro::VoidTask
    {
        utils::log::info("starting server loop");

        for (;;)
        {
            auto client = co_await server_socket.accept();
            read(std::move(client));
        }
    }

    std::tuple<Controllers...> controllers_;
};

}
