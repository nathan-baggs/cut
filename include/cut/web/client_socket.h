#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include <unistd.h>

#include "coro/event_loop.h"
#include "web/socket.h"

namespace cut::web
{

/**
 * Implementation of Socket for a connected client that can read bytes.
 */
class ClientSocket : public Socket
{
  public:
    /**
     * Construct a new ClientSocket.
     *
     *   @param fd
     *     The file descriptor for the socket.
     *   @param ev
     *     The event handler to resume us.
     */
    ClientSocket(int fd, coro::EventLoop &ev)
        : Socket(ev)
    {
        fd_ = {fd, ::close};
        ev_.register_socket(this);
    }

    /**
     * Read bytes from the network until a certain sequence is seen. This waits for some data to be available then
     * blocking reads until the sequence is found.
     *
     * @param end
     *   The string to wait for.
     */
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

                // keep reading till we get what we want
                while (!buffer.ends_with(end))
                {
                    auto c = char{};
                    if (::read(self.fd_, &c, sizeof(c)) != 1)
                    {
                        throw std::runtime_error("failed to read bytes");
                    }

                    buffer.push_back(c);
                }

                // return the read sting (including the ending)
                return buffer;
            }

            ClientSocket &self;
            std::string_view end;
        };

        return Awaitable{*this, end};
    }

    /**
     * Read a fixed number of bytes from the network. Like above this waits for some data to be available and then
     * blocks until all is read.
     *
     * @param num_bytes
     *   The number of bytes to read.
     */
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

    /**
     * Synchronous write - no coroutines here.
     *
     * @param data
     *   The data to send back.
     */
    auto write(const std::string data) -> void
    {
        const auto write_amount = ::write(fd_, data.data(), data.size());
        utils::ensure(static_cast<std::size_t>(write_amount) == data.size(), "failed to write response");
    }
};
}
