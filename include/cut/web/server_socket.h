#pragma once

#include <coroutine>
#include <cstdint>
#include <memory>
#include <stdexcept>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "coro/event_loop.h"
#include "utils/error.h"
#include "utils/log.h"
#include "web/socket.h"

namespace cut::web
{

class ServerSocket : public Socket
{
  public:
    ServerSocket(std::uint16_t port, coro::EventLoop &ev)
        : Socket(ev)
    {
        fd_ = {::socket(AF_INET, SOCK_STREAM, 0), ::close};
        utils::ensure(fd_, "could not create socket");

        auto opt = 0;
        const auto set_opt_res = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
        utils::ensure(set_opt_res != -1, "could not set socket options");

        auto addr = sockaddr_in{
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr = {.s_addr = ::inet_addr("127.0.0.1")},
            .sin_zero = {}};

        const auto bind_res = ::bind(fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
        utils::ensure(bind_res != -1, "could not bind");

        const auto listen_res = ::listen(fd_, 10);
        utils::ensure(listen_res != -1, "could not listen");

        const auto fcntl_res = ::fcntl(fd_, F_SETFL, O_NONBLOCK);
        utils::ensure(fcntl_res != -1, "could not set non-blocking");

        ev_.register_socket(this);
    }

    auto accept()
    {
        struct Awaitable
        {
            bool await_ready()
            {
                return false;
            }

            bool await_suspend(std::coroutine_handle<> handle)
            {
                utils::log::debug("await suspend");
                self.handle_ = handle;
                return true;
            }

            auto await_resume()
            {
                utils::log::debug("await resume");
                ::sockaddr_in addr{};
                ::socklen_t len = sizeof(addr);

                const auto client = ::accept(self.fd_, reinterpret_cast<sockaddr *>(&addr), &len);
                return client;
            }

            ServerSocket &self;
        };

        return Awaitable{*this};
    }
};

}
