#pragma once

#include <csignal>
#include <cstddef>
#include <map>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

#include "coro/event_loop.h"
#include "coro/void_task.h"
#include "utils/formatter.h"
#include "utils/log.h"
#include "utils/type_list.h"
#include "web/request.h"
#include "web/server_socket.h"
#include <sys/socket.h>

using namespace std::literals;

namespace cut::web
{

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
        static constexpr auto port = std::uint16_t{6375};

        utils::visit(controllers_, [](auto &&controller) { utils::log::info("{} registered", controller.name()); });

        utils::log::info("running on localhost:{}", port);

        auto ev = coro::EventLoop{};
        auto server_socket = ServerSocket{port, ev};

        accept(server_socket);
        ev.run();
    }

  private:
    auto handle_request(const Request &request) -> std::string
    {
        utils::log::info("handling {}", request);

        auto response_str = std::string{};

        utils::visit(
            controllers_,
            [&response_str, &request](auto &&controller)
            {
                if (controller.name() == request.controller)
                {
                    if (const auto response = controller.dispatch_handler(request); response)
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
            throw std::runtime_error(std::format("failed to handle request: {}", request));
        }

        return response_str;
    }

    auto read(std::unique_ptr<ClientSocket> client_socket) -> coro::VoidTask
    {
        utils::log::info("new client: {}", client_socket->native_handle());

        for (;;)
        {
            auto headers = std::map<std::string, std::string>{};

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

                headers.emplace(
                    header.substr(0u, colon_index),
                    header.substr(colon_index + 1u, header.length() - colon_index - 3u));
            }

            auto body = std::optional<std::string>{};

            if (const auto content_length = headers.find("Content-Length"); content_length != std::cend(headers))
            {
                body = co_await client_socket->read(std::stoi(content_length->second));
            }

            const auto request_line_parts =
                request_line | std::views::split(' ') |
                std::views::transform([](const auto &e)
                                      { return std::string(std::ranges::cbegin(e), std::ranges::cend(e)); }) |
                std::ranges::to<std::vector>();
            if (std::ranges::size(request_line_parts) != 3)
            {
                throw std::runtime_error("invalid request line");
            }

            const auto route_parts =
                request_line_parts[1] | std::views::split('/') |
                std::views::transform([](const auto &e)
                                      { return std::string(std::ranges::cbegin(e), std::ranges::cend(e)); }) |
                std::ranges::to<std::vector>();
            if (std::ranges::size(route_parts) != 3)
            {
                throw std::runtime_error("invalid request line (route)");
            }

            const auto response_str = handle_request(
                {.method = request_line_parts[0],
                 .controller = std::ranges::data(route_parts[1]),
                 .route = std::ranges::data(route_parts[2]),
                 .headers = std::move(headers),
                 .body = std::move(body)});

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

    utils::TypeList<Controllers...> controllers_;
};

}
