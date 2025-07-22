#pragma once

#include <csignal>
#include <cstddef>
#include <format>
#include <map>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "coro/event_loop.h"
#include "coro/void_task.h"
#include "cut/di/injector.h"
#include "db/db.h"
#include "db/sqlite3_db.h"
#include "utils/formatter.h"
#include "utils/log.h"
#include "utils/type_list.h"
#include "web/api.h"
#include "web/request.h"
#include "web/response.h"
#include "web/server_socket.h"

using namespace std::literals;

namespace cut::web
{
namespace details
{

// helper struct to visit each Controller (in a TypeList), construct it and pass it to a callback
template <class T>
struct Visitor;

// template recursion
template <class Head, class... Tail>
struct Visitor<utils::TypeList<Head, Tail...>>
{
    template <class I, class F>
    static auto visit(const I &injector, F &&f)
    {
        // use the injector to create
        f(injector.template create<Head>());

        Visitor<utils::TypeList<Tail...>>::visit(injector, std::forward<F>(f));
    }
};

// base case
template <class Head>
struct Visitor<utils::TypeList<Head>>
{
    template <class I, class F>
    static auto visit(const I &injector, F &&f)
    {
        // use the injector to create
        f(injector.template create<Head>());
    }
};

/**
 * Helper function to wrap up the above template misery.
 *
 * @param injector
 *   The injector to use for object creation (performs dependency injection).
 * @param f
 *   The function to pass each instantiated Controller to.
 */
template <class F, class I, class... Ts>
auto visit(utils::TypeList<Ts...>, const I &injector, F &&f)
{
    Visitor<utils::TypeList<Ts...>>::visit(injector, std::forward<F>(f));
}

}

/**
 * Entry point to the web server. Template with your controllers and then call run() (non-returning).
 */
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

    /**
     * Start the web server. Does not return.
     */
    auto run()
    {
        // hard coded port, classic
        static constexpr auto port = std::uint16_t{6375};

        // log the controllers
        details::visit(
            controllers_, injector_, [](auto &&controller) { utils::log::info("{} registered", controller.name()); });

        utils::log::info("running on localhost:{}", port);

        // setup the event loop and fire off the first coroutine

        auto ev = coro::EventLoop{};
        auto server_socket = ServerSocket{port, ev};

        accept(server_socket);
        ev.run();
    }

  private:
    /**
     * Handle a request and format into a response string.
     *
     * @param request
     *   The request to process.
     *
     * @returns
     *   The response formatted as a string (404 if no route can be found).
     */
    auto handle_request(const Request &request) -> std::string
    {
        utils::log::info("handling {}", request);

        auto response = std::optional<Response>{};

        // hard-coded route for the OpenAPI json endpoint
        if (request.method == "GET" && request.controller == "api" && request.route == "json")
        {
            response = json_api<Controllers...>();
        }
        else
        {
            // try and find a controller and matching function for the request
            // annoyingly due to how the visitor works we can't early exit, so we'll try all registered controllers even
            // if we find a match
            details::visit(
                controllers_,
                injector_,
                [&response, &request](auto &&controller)
                {
                    if (controller.name() == request.controller)
                    {
                        response = controller.dispatch_handler(request);
                    }
                });
        }

        // format the response if we got one, or fire back a 404
        return format_response(response.value_or(NotFound(std::format("/{}/{}", request.controller, request.route))));
    }

    /**
     * Coroutine to read a request from a client.
     *
     * @param client_socket
     *   The client socket to read from.
     */
    auto read(std::unique_ptr<ClientSocket> client_socket) -> coro::VoidTask
    {
        utils::log::info("new client: {}", client_socket->native_handle());

        auto headers = std::map<std::string, std::string>{};

        // get the request line
        const auto request_line = co_await client_socket->read_until("\r\n"sv);

        // get all the headers, not that we do much with them anyway
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
                header.substr(0u, colon_index), header.substr(colon_index + 1u, header.length() - colon_index - 3u));
        }

        auto body = std::optional<std::string>{};

        // get the body if there is one
        if (const auto content_length = headers.find("Content-Length"); content_length != std::cend(headers))
        {
            body = co_await client_socket->read(std::stoi(content_length->second));
        }

        // split and validate the request line
        const auto request_line_parts =
            request_line | std::views::split(' ') |
            std::views::transform([](const auto &e)
                                  { return std::string(std::ranges::cbegin(e), std::ranges::cend(e)); }) |
            std::ranges::to<std::vector>();
        if (std::ranges::size(request_line_parts) != 3)
        {
            throw std::runtime_error("invalid request line");
        }

        // split and validate the routes, sure hope the request conforms to our assumptions
        const auto route_parts =
            request_line_parts[1] | std::views::split('/') |
            std::views::transform([](const auto &e)
                                  { return std::string(std::ranges::cbegin(e), std::ranges::cend(e)); }) |
            std::ranges::to<std::vector>();
        if (std::ranges::size(route_parts) != 3)
        {
            throw std::runtime_error("invalid request line (route)");
        }

        // munge all the data into a request and hand it off for processing
        const auto response_str = handle_request(
            {.method = request_line_parts[0],
             .controller = std::ranges::data(route_parts[1]),
             .route = std::ranges::data(route_parts[2]),
             .headers = std::move(headers),
             .body = std::move(body)});

        utils::log::info("sending response: {}", response_str);

        // got a response - send it back (currently not async)
        client_socket->write(response_str);
    }

    /**
     * Coroutine to handle accepting clients.
     *
     * @param server_socket
     *   The server socket to listen for clients on.
     */
    auto accept(ServerSocket &server_socket) -> coro::VoidTask
    {
        utils::log::info("starting server loop");

        // loop forever accepting sockets
        for (;;)
        {
            auto client = co_await server_socket.accept();
            read(std::move(client));
        }
    }

    // we don't really need this make it makes the visitor slightly more convenient
    utils::TypeList<Controllers...> controllers_;

    /** Dependency injector - ironically this should be dependency injected into App, but hard-coded for now. */
    di::Injector<utils::TypeList<db::Db>, utils::TypeList<db::Sqlite3Db>> injector_;
};

}
