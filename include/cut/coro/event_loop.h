#pragma once

#include <vector>

namespace cut::web
{
class Socket;
}

namespace cut::coro
{

class EventLoop
{
  public:
    auto run() -> void;

    auto register_socket(web::Socket *socket) -> void
    {
        sockets_.push_back(socket);
    }

    auto unregister_socket(web::Socket *socket) -> void
    {
        to_remove_.push_back(socket);
    }

  private:
    std::vector<web::Socket *> sockets_;
    std::vector<web::Socket *> to_remove_;
};

}
