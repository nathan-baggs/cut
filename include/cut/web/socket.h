#pragma once

#include <coroutine>
#include <ranges>

#include <unistd.h>

#include "coro/event_loop.h"
#include "utils/auto_release.h"
#include "utils/error.h"
#include "utils/log.h"

namespace cut::web
{

class Socket
{
  public:
    Socket(coro::EventLoop &ev)
        : fd_{-1, ::close}
        , ev_{ev}
        , handle_{}
    {
    }

    virtual ~Socket()
    {
        if (fd_)
        {
            ev_.unregister_socket(this);
        }
    }

    Socket(const Socket &) = delete;
    auto operator=(const Socket &) -> Socket & = delete;
    Socket(Socket &&other) = default;

    auto native_handle() const -> int
    {
        return fd_;
    }

    auto resume() const -> void
    {
        utils::expect(!!handle_, "invalid handle");
        handle_.resume();
    }

  protected:
    utils::AutoRelease<int, -1> fd_;
    coro::EventLoop &ev_;
    std::coroutine_handle<> handle_;
};

}
