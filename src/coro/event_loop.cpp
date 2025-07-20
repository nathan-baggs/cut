#include "coro/event_loop.h"

#include <algorithm>
#include <ranges>

#include <sys/select.h>

#include "web/socket.h"

namespace cut::coro
{

auto EventLoop::run() -> void
{
    for (;;)
    {
        for (const auto remove : to_remove_)
        {
            std::erase(sockets_, remove);
        }

        ::fd_set fdset{};

        for (const auto *socket : sockets_)
        {
            FD_SET(socket->native_handle(), &fdset);
        }

        const auto max_socket = *std::ranges::max_element(
            sockets_, [](const auto *e1, const auto *e2) { return e1->native_handle() < e2->native_handle(); });

        ::select(max_socket->native_handle() + 1u, &fdset, nullptr, nullptr, nullptr);

        for (auto *socket : sockets_)
        {
            if (FD_ISSET(socket->native_handle(), &fdset) != 0)
            {
                socket->resume();
            }
        }
    }
}

}
