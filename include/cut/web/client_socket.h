#pragma once

#include <cstddef>
#include <vector>

#include <unistd.h>

#include "coro/event_loop.h"
#include "web/socket.h"

namespace cut::web
{

class ClientSocket : public Socket
{
  public:
    ClientSocket(int fd, coro::EventLoop &ev)
        : Socket(ev)
    {
        fd_ = {fd, ::close};
        ev_.register_socket(this);
    }

    auto read(std::size_t num_bytes)
    {
        struct Awaitable
        {
            bool await_ready()
            {
                return false;
            }

            bool await_suspend(std::coroutine_handle<> handle)
            {
                self.handle_ = handle;
                return true;
            }

            auto await_resume()
            {
                auto buffer = std::vector<char>(num_bytes);

                const auto read_length = ::read(self.fd_, buffer.data(), buffer.size());
                buffer.resize(read_length);

                return buffer;
            }

            ClientSocket &self;
            std::size_t num_bytes;
        };

        return Awaitable{*this, num_bytes};
    }
};
}
