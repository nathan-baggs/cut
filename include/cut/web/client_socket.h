#pragma once

#include <cstddef>
#include <string_view>
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

    auto read_until(std::string_view end)
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
                auto buffer = std::string{};

                while (!buffer.ends_with(end))
                {
                    auto c = char{};
                    if (::read(self.fd_, &c, sizeof(c)) != 1)
                    {
                        throw std::runtime_error("failed to read bytes");
                    }

                    buffer.push_back(c);
                }

                return buffer;
            }

            ClientSocket &self;
            std::string_view end;
        };

        return Awaitable{*this, end};
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

                return std::string(buffer.data(), buffer.data() + buffer.size());
            }

            ClientSocket &self;
            std::size_t num_bytes;
        };

        return Awaitable{*this, num_bytes};
    }

    auto write(const std::string data) -> void
    {
        const auto write_amount = ::write(fd_, data.data(), data.size());
        utils::ensure(static_cast<std::size_t>(write_amount) == data.size(), "failed to write response");
    }
};
}
